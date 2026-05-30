import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import remote_transmitter
from esphome.const import CONF_ID, CONF_QOS, CONF_TOPIC

AUTO_LOAD = ["remote_base"]
DEPENDENCIES = ["mqtt", "remote_transmitter"]

kelon168_mqtt_ir_ns = cg.esphome_ns.namespace("kelon168_mqtt_ir")
Kelon168MqttIr = kelon168_mqtt_ir_ns.class_("Kelon168MqttIr", cg.Component)

CONF_TRANSMITTER_ID = "transmitter_id"
CONF_SEND_TIMES = "send_times"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kelon168MqttIr),
        cv.Required(CONF_TRANSMITTER_ID): cv.use_id(remote_transmitter.RemoteTransmitterComponent),
        cv.Required(CONF_TOPIC): cv.string_strict,
        cv.Optional(CONF_QOS, default=0): cv.int_range(min=0, max=2),
        cv.Optional(CONF_SEND_TIMES, default=1): cv.int_range(min=1, max=5),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    transmitter = await cg.get_variable(config[CONF_TRANSMITTER_ID])
    cg.add(var.set_transmitter(transmitter))
    cg.add(var.set_topic(config[CONF_TOPIC]))
    cg.add(var.set_qos(config[CONF_QOS]))
    cg.add(var.set_send_times(config[CONF_SEND_TIMES]))
