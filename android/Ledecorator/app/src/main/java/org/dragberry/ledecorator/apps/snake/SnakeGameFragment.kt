package org.dragberry.ledecorator.apps.snake

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Switch
import org.dragberry.ledecorator.R
import org.dragberry.ledecorator.apps.AbstractAppFragment
import org.dragberry.ledecorator.bluetooth.Commands

private const val TAG = "SnakeGameFragment"

private const val SNAKE_GAME_SUB_FRAGMENT = "SnakeGameSubFragment"

class SnakeGameFragment : AbstractAppFragment(TAG) {

    private lateinit var autoManualSwitch: Switch

    enum class Mode(val value: Byte) {
        AUTO('A'.toByte()),
        MANUAL('M'.toByte());

        companion object {
            fun valueOf(code: Byte): Mode = values().find { it.value == code } ?: AUTO
        }
    }

    @Volatile
    private var mode = Mode.AUTO

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_snake_game, container, false)
        return view?.apply {
            autoManualSwitch = findViewById<Switch>(R.id.snakeAutoManualModeSwitch).apply {
                text = getString(R.string.ledecorator_app_mode_auto)
                setOnCheckedChangeListener { _, state ->
                    if (state) {
                        showManual()
                    } else {
                        showAuto()
                    }
                }
            }
            showAuto()
        }
    }

    private fun showManual() {
        childFragmentManager
            .beginTransaction()
            .replace(
                R.id.snakeGameFragmentLayout,
                SnakeGameManualFragment(),
                SNAKE_GAME_SUB_FRAGMENT
            )
            .commit()
        mode = Mode.MANUAL
        autoManualSwitch.text = getString(R.string.ledecorator_app_mode_manual)
    }

    private fun showAuto() {
        childFragmentManager
            .beginTransaction()
            .replace(
                R.id.snakeGameFragmentLayout,
                SnakeGameSettingsFragment(),
                SNAKE_GAME_SUB_FRAGMENT
            )
            .commit()
        mode = Mode.AUTO
        autoManualSwitch.text = getString(R.string.ledecorator_app_mode_auto)
    }

    override fun onDataFrame(bytes: ByteArray): ByteArray {
        return Commands.App.SNAKE.frame
    }
}