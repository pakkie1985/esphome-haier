import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_NAME

# Namespace en class verwijzing naar je C++ HaierV2
haier_ns = cg.esphome_ns.namespace('haier_v2')
HaierV2 = haier_ns.class_('HaierV2', climate.Climate, cg.PollingComponent)

# Config schema voor ESPHome
CONFIG_SCHEMA = climate.CLIMATE_SCHEMA.extend({
    cv.Required(CONF_NAME): cv.string,
}).extend(climate.CLIMATE_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_NAME])
    cg.add(var)
    climate.setup_climate(var, config)
