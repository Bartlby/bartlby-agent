AUTOMAKE_OPTIONS = subdir-objects foreign
ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}


noinst_PROGRAMS = bartlby_agent bartlby_agent_v2 bartlby_cmd

bartlby_agent_SOURCES = config.c  agent.c
bartlby_agent_LDADD = -ldl $(OPENSSL_FLAG) $(SNMP_FLAG)
bartlby_agent_CFLAGS = $(AM_CFLAGS)  $(NRPE_ADDON) $(OPENSSL_ADDON)  $(SNMP_ADDON)


bartlby_agent_v2_SOURCES = config.c  agent_v2.c global.c
bartlby_agent_v2_LDADD = -ldl $(OPENSSL_FLAG) $(SNMP_FLAG)
bartlby_agent_v2_CFLAGS = $(AM_CFLAGS)  $(NRPE_ADDON) $(OPENSSL_ADDON)  $(SNMP_ADDON)


bartlby_cmd_SOURCES = config.c  global.c cmd.c
bartlby_cmd_LDADD = -ldl $(OPENSSL_FLAG) $(SNMP_FLAG)
bartlby_cmd_CFLAGS = $(AM_CFLAGS)  $(NRPE_ADDON) $(OPENSSL_ADDON)  $(SNMP_ADDON)







mysqldir=$(prefix)
dist_mysql_DATA=bartlby.cfg agent_sync.cfg agent_sync.sh passive_run.sample HOWTO.passive bartlby_agent bartlby_agent_v2 bartlby_cmd





dist_noinst_SCRIPTS = autogen.sh

install-data-hook:
	chmod +x  $(DESTDIR)$(prefix)/bartlby_agent
	chmod +x  $(DESTDIR)$(prefix)/bartlby_agent_v2
	chmod +x  $(DESTDIR)$(prefix)/bartlby_cmd


dist-hook:
	make clean
