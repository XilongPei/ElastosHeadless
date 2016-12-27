set(conf_dir "${MK_PATH_CONF}/plugins/ElastosSuperExe/")

install(DIRECTORY DESTINATION ${conf_dir})

if(BUILD_LOCAL)
  file(COPY ElastosSuperExe.conf DESTINATION ${conf_dir})
  file(COPY themes DESTINATION ${conf_dir})
else()
  install(FILES ElastosSuperExe.conf DESTINATION ${conf_dir})
  install(DIRECTORY themes DESTINATION ${conf_dir})
endif()
