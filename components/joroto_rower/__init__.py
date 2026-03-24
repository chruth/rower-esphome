from esphome import codegen as cg

rowing_monitor_ns = cg.esphome_ns.namespace("rowing_monitor")
RowingMonitorComponent = rowing_monitor_ns.class_("RowingMonitorComponent", cg.Component)
