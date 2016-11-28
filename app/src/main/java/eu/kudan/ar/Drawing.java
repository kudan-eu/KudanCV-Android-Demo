package eu.kudan.ar;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Typeface;

/**
 * Class containing static methods used to perform the drawing of primitive UI objects on a canvas.
 */
class Drawing {

    /**
     * Member class that draws an arbitrary quadrilateral shape with a centred text label on a
     * Canvas given four corner coordinates in screen-space.
     */
    private static class Quadrilateral {

        static Path mPath = new Path();

        static Paint mPathPaint = new Paint();
        static Paint mTextPaint = new Paint();

        /**
         * Constructs the Quadrilateral object and sets the appearance of the path and text.
         */
        static {

            mPathPaint.setStrokeWidth(10);
            mPathPaint.setPathEffect(null);
            mPathPaint.setStyle(Paint.Style.STROKE);

            mTextPaint.setColor(Color.CYAN);
            mTextPaint.setStyle(Paint.Style.FILL);
            mTextPaint.setTypeface(Typeface.SANS_SERIF);
            mTextPaint.setTextAlign(Paint.Align.CENTER);
            mTextPaint.setTextSize(48);
        }

        /**
         * Draws a Quadrilateral on a Canvas.
         *
         * @param canvas The Canvas object the Quadrilateral will be drawn on.
         * @param transform The matrix transform applied to the Canvas before drawing.
         * @param p0 The top-left corner of the Quadrilateral.
         * @param p1 The top-right corner of the Quadrilateral.
         * @param p2 The bottom-right corner of the Quadrilateral.
         * @param p3 The bottom-left corner of the Quadrilateral.
         * @param label The text that should be displayed in the centre of the Quadrilateral.
         * @param color The color of the Quadrilateral outline.
         */
        private static void draw(Canvas canvas, Matrix transform, Point p0, Point p1, Point p2, Point p3, String label, int color) {

            canvas.setMatrix(transform);

            // Reset path to prevent overdraw.
            mPath.reset();

            // Draw border path.
            mPath.moveTo(p0.x, p0.y);
            mPath.lineTo(p1.x, p1.y);
            mPath.lineTo(p2.x, p2.y);
            mPath.lineTo(p3.x, p3.y);

            mPath.close();

            mPathPaint.setColor(color);

            canvas.drawPath(mPath, mPathPaint);

            // Draw label text.
            canvas.drawText(
                    label,
                    (p2.x + p0.x) / 2,
                    (p2.y + p0.y - (mTextPaint.descent() + mTextPaint.ascent())) / 2,
                    mTextPaint
            );
        }
    }

    /**
     * Member class that draws an arbitrary quadrilateral shape with a centred text label and an
     * interior grid on a Canvas given four corner coordinates in screen-space.
     */
    private static class Grid extends Quadrilateral {

        /**
         * Draws a Grid on a Canvas.
         *
         * @param canvas The Canvas object the Grid will be drawn on.
         * @param transform The matrix transform applied to the Canvas before drawing.
         * @param p0 The top-left corner of the Grid.
         * @param p1 The top-right corner of the Grid.
         * @param p2 The bottom-right corner of the Grid.
         * @param p3 The bottom-left corner of the Grid.
         * @param label The text that should be displayed in the centre of the Grid.
         * @param color The color of the Quadrilateral Grid.
         */
        private static void draw(Canvas canvas, Matrix transform, Point p0, Point p1, Point p2, Point p3, String label, int color) {

            canvas.setMatrix(transform);

            // Reset path to prevent overdraw.
            mPath.reset();

            // Draw grid path.
            mPath.moveTo((p1.x + p0.x) / 2, (p1.y + p0.y) / 2);
            mPath.lineTo((p3.x + p2.x) / 2, (p3.y + p2.y) / 2);
            mPath.moveTo((p3.x + p0.x) / 2, (p3.y + p0.y) / 2);
            mPath.lineTo((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);

            mPathPaint.setColor(color);

            canvas.drawPath(mPath, mPathPaint);

            // Draw the quadrilateral border over the interior grid.
            Quadrilateral.draw(canvas, transform, p0, p1, p2, p3, label, color);
        }
    }

    /**
     * Member class that draws an alpha bitmap as a black and white image to fill a Canvas.
     */
    private static class Frame {

        /**
         * The background paint of the drawn image.
         */
        private static Paint bgPaint = new Paint();

        /**
         * The foreground paint of the drawn image.
         *
         * This will be masked by the alpha bitmap supplied during drawing.
         */
        private static Paint fgPaint = new Paint();

        /**
         * Rect object that controls the drawing area of the image.
         */
        private static Rect rect = new Rect();

        /**
         * Constructs the frame object.
         */
        static {

            bgPaint.setColor(Color.BLACK);

            fgPaint.setColor(Color.WHITE);

            // Set the alpha blending mode of the foreground paint to allow the bitmap to draw over
            // the background fill.
            fgPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
        }

        /**
         * Draws a Frame on a Canvas.
         *
         * @param canvas The Canvas the Frame will be drawn on.
         * @param bitmap The Bitmap that will be drawn. The supplied bitmap should be of format
         *               Bitmap.Config.ALPHA_8.
         */
        private static void draw(Canvas canvas, Bitmap bitmap) {

            // Set the drawing area to fill the Canvas.
            rect.set(0, 0, canvas.getWidth(), canvas.getHeight());

            // Draw the background.
            canvas.drawRect(rect, bgPaint);

            // Draw the foreground masked by the supplied alpha bitmap.
            canvas.drawBitmap(bitmap, null, rect, fgPaint);
        }
    }

    /**
     * Parameters used to specify the type of primitive to render during drawing.
     */
    enum DrawingPrimitive {
        DRAWING_RECTANGLE,
        DRAWING_GRID,
        DRAWING_NOTHING,
        DRAWING_BACKGROUND
    }

    /**
     * Draws a Primitive on a Canvas.
     * @param canvas The Canvas on which the Primitive should be drawn.
     * @param transform The matrix transform applied to primitive drawing.
     * @param primitive The primitive of type DrawingPrimitive to be drawn.
     * @param p0 The top-left corner of the Primitive.
     * @param p1 The top-right corner of the Primitive.
     * @param p2 The bottom-right corner of the Primitive.
     * @param p3 The bottom-left corner of the Primitive.
     * @param label The text that should be displayed in the centre of the Primitive.
     */
    static void drawPrimitive(Canvas canvas, Matrix transform, DrawingPrimitive primitive, Point p0, Point p1, Point p2, Point p3, String label) {

        if (primitive == DrawingPrimitive.DRAWING_RECTANGLE) {

            Quadrilateral.draw(canvas, transform, p0, p1, p2, p3, label, Color.BLUE);

        }
        else if (primitive == DrawingPrimitive.DRAWING_GRID) {

            Grid.draw(canvas, transform, p0, p1, p2, p3, label, Color.GREEN);

        }
    }

    /**
     * Draws an alpha bitmap to fill a Canvas as a black-and-white image.
     *
     * @param canvas The Canvas on which the bitmap should be drawn.
     * @param bitmap The Bitmap that will be drawn. The supplied bitmap should be of format
     *               Bitmap.Config.ALPHA_8.
     */
    static void drawBackground(Canvas canvas, Bitmap bitmap) {

        Frame.draw(canvas, bitmap);
    }
}
