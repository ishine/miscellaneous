project(gsm-efr-simulator)
aux_source_directory(. gsmefr-src)
add_library(gsm-efr-impl ${gsmefr-src})