set(srcs "main.cpp"
         "XboxControllerNotificationParser/src/XboxControllerNotificationParser.cpp"
         "esp_hid_gap.c")

set(include_dirs ".")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES esp_hid)
