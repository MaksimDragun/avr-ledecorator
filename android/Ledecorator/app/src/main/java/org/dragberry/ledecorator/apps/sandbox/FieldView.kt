package org.dragberry.ledecorator.apps.sandbox

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Matrix
import android.util.AttributeSet
import android.view.MotionEvent
import android.widget.ImageView
import org.dragberry.ledecorator.utils.Color
import org.dragberry.ledecorator.utils.Colors
import java.io.InputStream
import kotlin.math.roundToInt

const val FIELD_WIDTH = 32
const val FIELD_HEIGHT = 16
const val UPLOAD_BLOCK_SIZE = 12


class FieldView(context: Context, attributesSet: AttributeSet) :
    ImageView(context, attributesSet) {

    private var onDrawPoint: ((x: Int, y: Int, color: Color) -> Unit)? = null

    var currentColor: Color = Colors.WHITE

    private var bitmap: Bitmap = Bitmap.createBitmap(FIELD_HEIGHT, FIELD_WIDTH, Bitmap.Config.ARGB_8888).apply {
        setImageBitmap(this)
        eraseColor(Colors.BLACK.real)
    }

    private val inverseMatrix: Matrix = Matrix()
    private val points = FloatArray(2)

    fun getImage(handle: (bitmap: Bitmap) -> Unit) {
        val m = Matrix()
        m.postRotate(270.0f)
        val temp = Bitmap.createBitmap(bitmap, 0, 0, bitmap.width, bitmap.height, m, false)
        handle(temp)
        temp.recycle()
    }

    fun setImage(input: InputStream, onBlock: (x: Int, y: Int, size: Int, block: IntArray) -> Unit) {
        val options = BitmapFactory.Options()
        options.inPreferredConfig = Bitmap.Config.ARGB_8888
        BitmapFactory.decodeStream(input, null, options)?.apply {
            val m = Matrix()
            m.postScale(FIELD_WIDTH.toFloat() / width, FIELD_HEIGHT.toFloat() / height)
            m.postRotate(90.0f)
            bitmap = Bitmap.createBitmap(this, 0, 0, width, height, m, true)

            var block = IntArray(UPLOAD_BLOCK_SIZE)
            var blockX = 0
            var blockY = 0
            var blockProgress = 0
            for (y in 0 until FIELD_HEIGHT) {
                for (x in 0 until FIELD_WIDTH) {
                    if (blockProgress == UPLOAD_BLOCK_SIZE) {
                        onBlock(blockX, blockY, blockProgress, block)
                        block = IntArray(UPLOAD_BLOCK_SIZE)
                        blockProgress = 0
                        blockX = x
                        blockY = y
                    }

                    val realX = FIELD_HEIGHT - y - 1
                    val pixel = Colors.getByReal(bitmap.getPixel(realX, x))
                    bitmap.setPixel(realX, x, pixel.real)
                    block[blockProgress++] = pixel.display
                }
            }
            if (blockProgress > 0) {
                onBlock(blockX, blockY, blockProgress, block)
            }
            setImageBitmap(bitmap)
            recycle()
            postInvalidate()
        }
    }

    fun clearScreen(color: Color) {
        bitmap.eraseColor(color.real)
    }

    fun setPixel(x: Int, y: Int, color: Int) {
        bitmap.setPixel(FIELD_HEIGHT- y - 1, x, color)
    }

    fun onDrawPoint(onDrawPoint: (x: Int, y: Int, color: Color) -> Unit) {
        this.onDrawPoint = onDrawPoint
    }

    override fun performClick(): Boolean {
        super.performClick()
        return true
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (event == null) return true

        return when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                true
            }
            MotionEvent.ACTION_UP -> {
                performAction(event)
                performClick()
                true
            }
            MotionEvent.ACTION_MOVE -> {
                performAction(event)
                true
            }
            else -> true
        }
    }

    private fun performAction(event: MotionEvent) {
        imageMatrix.invert(inverseMatrix)
        points[0] = event.x
        points[1] = event.y
        inverseMatrix.mapPoints(points)
        val x = points[0].roundToInt()
        val y = points[1].roundToInt()

        if (y in 0 until FIELD_WIDTH && x in 0 until FIELD_HEIGHT) {
            val previousColor: Int = bitmap.getPixel(x, y)
            if (previousColor != currentColor.real) {
                onDrawPoint?.let { it(y, FIELD_HEIGHT - x - 1, currentColor) }
                bitmap.setPixel(x, y, currentColor.real)
                postInvalidate()
            }

        }
    }
}