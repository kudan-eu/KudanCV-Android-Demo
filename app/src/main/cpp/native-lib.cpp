#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>

#include <Interface.h>

/**
 * Helper method for projecting 3D tracking points to screen-space.
 */
KudanVector2 project(KudanVector3 X, KudanMatrix3 intrinsicMatrix, KudanVector3 position, KudanQuaternion orientation) {

    // to project a 3D point X (3x1) according to a camera with rotation R (3x3) and translation T (3x1), and the camera intrinsic matrkx K (3x3), xh = K[R|T]X = K*(RX + T), where xh is the homogeneous point


    //KudanMatrix3 rotationMatrix = KudanQuaternion::quaternionToRotation(orientation);
    KudanMatrix3 rotationMatrix(orientation);

    KudanVector3 RX = rotationMatrix * X;
    KudanVector3 RXplusT = RX + position; // this is the point X expressed in the camera's coordinate frame

    // Project using the intrinsicmatrix:
    KudanVector3 XH = intrinsicMatrix * RXplusT;

    // Divide the homogeneous coordinates through by the z coordinate
    KudanVector2 pt(XH.x / XH.z , XH.y / XH.z);

    return pt;
}

extern "C" {

static std::shared_ptr<KudanImageTracker> imageTracker;
static std::shared_ptr<KudanArbiTracker> arbiTracker;
static int arbitrackScale;

void Java_eu_kudan_ar_CameraFragment_initialiseImageTracker(
        JNIEnv *env,
        jobject /* this */,
        jstring key,
        jint width,
        jint height) {

    imageTracker = std::make_shared<KudanImageTracker>();

    // only track one at a time for now
    imageTracker->setMaximumSimultaneousTracking(1);


    // Setup the camera parameters
    KudanCameraParameters parameters;

    // Should use the camera to get the image stream size, but it's not initialised yet, so just hardcode for now
    parameters.setSize(width, height);

    // Don't know the intrinsics so tell it to figure them out
    parameters.guessIntrinsics();

    // Important: set the intrinsic parameters on the tracker
    imageTracker->setCameraParameters(parameters);

    // Set API key
    const char *keyStr = env->GetStringUTFChars(key, 0);
    std::string apiKey = std::string(keyStr);

    imageTracker->setApiKey(apiKey);

    env->ReleaseStringUTFChars(key, keyStr);
}

void Java_eu_kudan_ar_CameraFragment_initialiseArbiTracker(
        JNIEnv *env,
        jobject /* this */,
        jstring key,
        jint width,
        jint height) {

    arbiTracker = std::make_shared<KudanArbiTracker>();

    // Setup the camera parameters
    KudanCameraParameters parameters;

    // Should use the camera to get the image stream size, but it's not initialised yet, so just hardcode for now
    parameters.setSize(width, height);

    // Don't know the intrinsics so tell it to figure them out
    parameters.guessIntrinsics();

    // Important: set the intrinsic parameters on the tracker
    arbiTracker->setCameraParameters(parameters);

    // Set API key
    const char *keyStr = env->GetStringUTFChars(key, 0);
    std::string apiKey = std::string(keyStr);

    arbiTracker->setApiKey(apiKey);

    env->ReleaseStringUTFChars(key, keyStr);
}

void Java_eu_kudan_ar_CameraFragment_startArbiTracker(
        JNIEnv *env,
        jobject /* this */,
        jboolean startFromImageTrackable) {

    KudanVector3 startPosition;
    KudanQuaternion startOrientation;

    if (startFromImageTrackable) {

        std::vector<std::shared_ptr<KudanImageTrackable>> detectedTrackables = imageTracker->getDetectedTrackables();

        if (detectedTrackables.size() == 0) {
            throw std::runtime_error("Image trackable not detected");
        }

        std::shared_ptr<KudanImageTrackable> trackable = detectedTrackables[0];

        startPosition = trackable->getPosition();
        startOrientation = trackable->getOrientation();

        arbitrackScale = trackable->getHeight() / 2;
    }
    else {

        startPosition = KudanVector3(0,0,600); // in front of the camera
        startOrientation = KudanQuaternion(1,0,0,0); // without rotation

        arbitrackScale = 100;
    }

    arbiTracker->start(startPosition, startOrientation);
}

void Java_eu_kudan_ar_CameraFragment_stopArbiTracker(
        JNIEnv *env,
        jobject /* this */) {

    arbiTracker->stop();
}

jboolean Java_eu_kudan_ar_CameraFragment_addTrackableToImageTracker(
        JNIEnv *env,
        jobject /* this */,
        jobject bitmap,
        jstring name) {

    // Get the trackable image pixel data and info.
    AndroidBitmapInfo bitmapInfo;
    void *data;
    int result;

    result = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo);

    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        __android_log_print(ANDROID_LOG_ERROR, "JNI", "Could not get trackable image info.");
        return false;
    }

    result = AndroidBitmap_lockPixels(env, bitmap, &data);

    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        __android_log_print(ANDROID_LOG_ERROR, "JNI", "Could not lock trackable image pixels.");
        return false;
    }

    const char *nameStr = env->GetStringUTFChars(name, 0);

    // This is how to create a KudanImageTrackable object, using a pointer to image data:
    std::shared_ptr<KudanImageTrackable> kudanImageTrackable = KudanImageTrackable::createFromImageData(
            static_cast<unsigned char *>(data),
            nameStr,
            bitmapInfo.width,
            bitmapInfo.height,
            4,
            0
    );

    // Release all data held by JNI.
    env->ReleaseStringUTFChars(name, nameStr);

    result = AndroidBitmap_unlockPixels(env, bitmap);

    if (result != ANDROID_BITMAP_RESULT_SUCCESS) {
        __android_log_print(ANDROID_LOG_ERROR, "JNI", "Could not unlock trackable image pixels.");
    }

    // createFromImageData returns a null pointer if unsuccessful.
    if (kudanImageTrackable) {

        // Once the trackable is created, it needs to be added to the tracker!
        return imageTracker->addTrackable(kudanImageTrackable);
    }
    else {
        return false;
    }
}

jfloatArray Java_eu_kudan_ar_CameraFragment_processImageTrackerFrame(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray image,
        jint width,
        jint height,
        jint channels,
        jint padding,
        jboolean requireFlip) {

    jbyte *data = env->GetByteArrayElements(image, 0);

    unsigned char *base = (unsigned char *) data;

    imageTracker->processFrame(base, width, height, channels, padding, requireFlip);

    env->ReleaseByteArrayElements(image, data, JNI_ABORT);


    std::vector<std::shared_ptr<KudanImageTrackable>> trackedList = imageTracker->getDetectedTrackables();

    if (trackedList.size() == 1) {

        std::shared_ptr<KudanImageTrackable> tracked = trackedList[0];

        float trackedData[10];

        /** Get the pose of the tracked object to draw it
             This is expressed as a 3D position and a unit quaternion for orientation
             This is: the position of the trackable centre with respect to the camera, and the orientation of the trackable about this centre
             This is equivalent to having the rotation (R) and translation (T) of a camera with respect to the trackable, with which the marker position (in its own coordinate frame) can be projected to the image
             */


        // To project the tracked marker centre into the image, use the marker centre in its own coodinate frame (obviously the origin) and project that using the tracked pose expressed as a camera */
        KudanVector3 origin(0,0,0);

        // Get the camera intrinsics as a 3x3 matrix
        KudanMatrix3 K = imageTracker->getCameraMatrix();

        // Project the point (0,0,0) using the camera intrinsics and extrinsics. Also need to pass in the image with (see function)
        KudanVector3 position = tracked->getPosition();
        KudanQuaternion orientation = tracked->getOrientation();
        KudanVector2 projection = project(origin, K, position, orientation);



        // Store the details of this trackable
        trackedData[0] = projection.x;
        trackedData[1] = projection.y;


        // As well as the centre, it's useful to draw the four corners of the tracked object
        // Because the trackable is defined as having size width x height in the world, and is at the origin (of its own coordinate frame), the bounds of the marker are at (+/- width/2, +/- height/2). Projecting these four points into the current image using the camera pose gets the outline of the tracked marker in the image:

        float w = tracked->getWidth();
        float h = tracked->getHeight();



        KudanVector3 corner00(-w/2.f, -h/2.f, 0);
        KudanVector2 projection00 = project(corner00, K, position, orientation);

        KudanVector3 corner01(-w/2.f, h/2.f, 0);
        KudanVector2 projection01 = project(corner01, K, position, orientation);

        KudanVector3 corner11(w/2.f, h/2.f, 0);
        KudanVector2 projection11 = project(corner11, K, position, orientation);

        KudanVector3 corner10(w/2.f, -h/2.f, 0);
        KudanVector2 projection10 = project(corner10, K, position, orientation);

        trackedData[2] = projection00.x;
        trackedData[3] = projection00.y;
        trackedData[4] = projection01.x;
        trackedData[5] = projection01.y;
        trackedData[6] = projection11.x;
        trackedData[7] = projection11.y;
        trackedData[8] = projection10.x;
        trackedData[9] = projection10.y;

        jfloatArray result = env->NewFloatArray(10);

        env->SetFloatArrayRegion(result, 0, 10, trackedData);

        return result;
    }

    return NULL;
}

jfloatArray Java_eu_kudan_ar_CameraFragment_processArbiTrackerFrame(
        JNIEnv *env,
        jobject /* this */,
        jbyteArray image,
        jfloatArray gyroOrentation,
        jint width,
        jint height,
        jint channels,
        jint padding,
        jboolean requireFlip) {

    jbyte *data = env->GetByteArrayElements(image, 0);
    jfloat *orientation = env->GetFloatArrayElements(gyroOrentation, 0);
    
    // Important: before calling processFrame on Arbitrack, it is necessary to provide an orientation estimate from some other sensor, e.g. Android IMU
    // If this is not done, then Arbitrack will not output an orientation
    
    // Use this function to set the orientation:
    // KudanQuaternion constructor takes values in (x,y,z,w) order, so we compensate.
    KudanQuaternion gyroQuaternion = KudanQuaternion(orientation[1], orientation[2], orientation[3], orientation[0]);

    arbiTracker->setSensedOrientation( gyroQuaternion );
    
    unsigned char *base = (unsigned char *) data;

    arbiTracker->processFrame(base, width, height, 1 /* assume one channel*/, padding, false /* don't need to flip the image*/);

    env->ReleaseByteArrayElements(image, data, JNI_ABORT);

    if (arbiTracker->isTracking()) {

        float trackedData[10];

        // Get the camera intrinsics as a 3x3 matrix
        KudanMatrix3 K = arbiTracker->getCameraMatrix(); // need this on arbitracker - oops! TODO


        KudanVector3 position = arbiTracker->getPosition();
        // make sure it's not the zero vector
        if (position.x == 0 && position.y == 0 && position.z == 0) {

            for (int i = 0;i < 10;i++) {
                trackedData[i] = 0.0f;
            }

            jfloatArray result = env->NewFloatArray(10);

            env->SetFloatArrayRegion(result, 0, 10, trackedData);

            return result;
        }
        else {
            KudanQuaternion orientation = arbiTracker->getOrientation();


            KudanVector3 origin(0,0,0);

            KudanVector2 projection = project(origin, K, position, orientation);

            trackedData[0] = projection.x;
            trackedData[1] = projection.y;

            // Get the four outer grid corners by projecting  +/- the arbitrack scale in (x,y)
            KudanVector3 corner00(-arbitrackScale, -arbitrackScale, 0);
            KudanVector2 projection00 = project(corner00, K, position, orientation);

            KudanVector3 corner01(-arbitrackScale, arbitrackScale, 0);
            KudanVector2 projection01 = project(corner01, K, position, orientation);

            KudanVector3 corner11(arbitrackScale, arbitrackScale, 0);
            KudanVector2 projection11 = project(corner11, K, position, orientation);

            KudanVector3 corner10(arbitrackScale, -arbitrackScale, 0);
            KudanVector2 projection10 = project(corner10, K, position, orientation);


            // Save as four separate points as properties on the MarkerTracker:
            trackedData[2] = projection00.x;
            trackedData[3] = projection00.y;
            trackedData[4] = projection01.x;
            trackedData[5] = projection01.y;
            trackedData[6] = projection11.x;
            trackedData[7] = projection11.y;
            trackedData[8] = projection10.x;
            trackedData[9] = projection10.y;

            jfloatArray result = env->NewFloatArray(10);

            env->SetFloatArrayRegion(result, 0, 10, trackedData);

            return result;
        }

    }

    return NULL;
}

} // extern "C"
