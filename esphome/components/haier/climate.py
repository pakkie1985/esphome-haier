import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_NAME

# Namespace voor je component
haier_ns = cg.esphome_ns.namespace('haier')
HaierV2 = haier_ns.class_('Haier')  # hier exact de C++ klasse naam

CONFIG_SCHEMA = climate.climate_schema(HaierV2).extend({
    cv.Required(CONF_NAME): cv.string,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_NAME], HaierV2)  # <-- hier de klasse meegeven
    cg.add(var)
    await climate.register_climate(var, config)
