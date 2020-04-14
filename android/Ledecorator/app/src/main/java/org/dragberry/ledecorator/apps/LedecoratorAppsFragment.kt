package org.dragberry.ledecorator.apps

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import kotlinx.android.synthetic.main.fragment_ledecorator_apps_item.view.*
import org.dragberry.ledecorator.BluetoothServiceHolder
import org.dragberry.ledecorator.MainActivity
import org.dragberry.ledecorator.R
import org.dragberry.ledecorator.bluetooth.DataFrames

private const val TAG = "LedecoratorAppFragment"

private const val FRAME_RECEIVED = 3000

class LedecoratorAppFragment(private val onAppSelectedListener: (LedecoratorApp.() -> Unit)) :
    Fragment(), Handler.Callback {

    private var bluetoothService: MainActivity.BluetoothService? = null

    private val ledecoratorAppRecyclerViewAdapter: LedecoratorAppsRecyclerViewAdapter =
        LedecoratorAppsRecyclerViewAdapter(LedecoratorApps.APPS)

    private var selectedAppIndex: Int = -1

    private var handler: Handler? = null

    override fun onAttach(context: Context) {
        super.onAttach(context)
        handler = Handler(this)
        if (context is BluetoothServiceHolder) {
            bluetoothService = context.bluetoothService
            bluetoothService?.onDataFrame(TAG) {
                if (DataFrames.check(this)) {
                    LedecoratorApps.APPS.forEachIndexed { i, it ->
                        it.active = it.code == get(1)
                        it.selected = i == selectedAppIndex
                    }
                    handler?.apply {
                        obtainMessage(FRAME_RECEIVED).sendToTarget()
                    }
                }
                bluetoothService?.responseDataFrame = DataFrames.IDLE
            }
        } else {
            throw RuntimeException("$context must implement ${BluetoothServiceHolder::javaClass.name}")
        }
    }

    override fun onDetach() {
        super.onDetach()
        bluetoothService?.onDataFrame(TAG, null)
        bluetoothService = null
        handler?.removeCallbacksAndMessages(null)
        handler = null
    }

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        val view = inflater.inflate(R.layout.fragment_ledecorator_apps, container, false)
        if (view != null) {
            view.findViewById<RecyclerView>(R.id.fragment_ledecorator_apps_list)?.apply {
                layoutManager = LinearLayoutManager(context)
                adapter = ledecoratorAppRecyclerViewAdapter
            }
        }
        return view
    }

    override fun handleMessage(msg: Message): Boolean {
        when (msg.what) {
            FRAME_RECEIVED -> {
                ledecoratorAppRecyclerViewAdapter.notifyDataSetChanged()
            }
        }
        return true
    }

    inner class LedecoratorAppsRecyclerViewAdapter(private val appList: List<LedecoratorApp>) :
        ListAdapter<LedecoratorApp, LedecoratorAppsRecyclerViewAdapter.ViewHolder>(DiffCallback()) {

        private var selectedIndex: Int = -1

        init {
            submitList(appList)
        }

        inner class ViewHolder(val view: View) : RecyclerView.ViewHolder(view) {
            val appNameTextView: TextView = view.ledecoratorAppNameTextView
            val appCodeTextView: TextView = view.ledecoratorAppCodeTextView
            val appStatusTextView: TextView = view.ledecoratorAppStatusTextView
        }

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder =
            ViewHolder(LayoutInflater.from(parent.context).inflate(R.layout.fragment_ledecorator_apps_item, parent, false))

        override fun onBindViewHolder(holder: ViewHolder, position: Int) {
            val item = appList[position]
            holder.appNameTextView.text = item.name
            holder.appCodeTextView.text = "0x0${item.code.toString(16)}"
            holder.appStatusTextView.apply {
                if (item.active) {
                    text = getString(R.string.ledecorator_app_status_active)
                    visibility = View.VISIBLE
                } else {
                    text = getString(R.string.ledecorator_app_status_active)
                    visibility = View.INVISIBLE
                }
            }

            holder.apply {
                view.setBackgroundColor(if (selectedIndex == holder.adapterPosition) Color.GREEN else Color.WHITE)
                view.setOnClickListener {
                    if (selectedIndex != holder.adapterPosition) {
                        notifyItemChanged(selectedIndex)
                        selectedIndex = holder.adapterPosition
                    } else {
                        onAppSelectedListener.invoke(item)
                    }
                }
            }
        }
    }

    private class DiffCallback : DiffUtil.ItemCallback<LedecoratorApp>() {

        override fun areItemsTheSame(oldItem: LedecoratorApp, newItem: LedecoratorApp): Boolean {
            return oldItem.code == newItem.code
        }

        override fun areContentsTheSame(oldItem: LedecoratorApp, newItem: LedecoratorApp): Boolean {
            return oldItem.code == newItem.code
        }
    }
}
