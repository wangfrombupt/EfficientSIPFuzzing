checker.o checker.d : checker.c ../scscf/ifc_datastruct.h ../scscf/registrar.h \
 ../scscf/../../sr_module.h ../scscf/../../parser/msg_parser.h \
 ../scscf/../../parser/../comp_defs.h ../scscf/../../parser/../str.h \
 ../scscf/../../parser/../lump_struct.h \
 ../scscf/../../parser/.././parser/hf.h \
 ../scscf/../../parser/.././parser/../str.h \
 ../scscf/../../parser/.././parser/../comp_defs.h \
 ../scscf/../../parser/../flags.h ../scscf/../../parser/../ip_addr.h \
 ../scscf/../../parser/../str.h ../scscf/../../parser/../dprint.h \
 ../scscf/../../parser/../md5utils.h ../scscf/../../parser/../config.h \
 ../scscf/../../parser/../types.h ../scscf/../../parser/parse_def.h \
 ../scscf/../../parser/parse_cseq.h ../scscf/../../parser/parse_to.h \
 ../scscf/../../parser/parse_via.h ../scscf/../../parser/parse_fline.h \
 ../scscf/../../parser/hf.h ../scscf/../../parser/../error.h \
 ../scscf/../../version.h ../scscf/../../rpc.h \
 ../scscf/../../route_struct.h ../scscf/../../select.h \
 ../scscf/../../str.h ../scscf/../../usr_avp.h ../scscf/../../locking.h \
 ../scscf/../../lock_ops.h ../scscf/../../fastlock.h \
 ../scscf/../../lock_alloc.h ../scscf/../../mem/mem.h \
 ../scscf/../../mem/../config.h ../scscf/../../mem/../dprint.h \
 ../scscf/../../mem/q_malloc.h ../scscf/../../mem/meminfo.h \
 ../scscf/../../mem/shm_mem.h ../scscf/../../mem/../lock_ops.h \
 /usr/include/libxml2/libxml/xmlschemas.h \
 /usr/include/libxml2/libxml/xmlversion.h \
 /usr/include/libxml2/libxml/xmlexports.h \
 /usr/include/libxml2/libxml/tree.h \
 /usr/include/libxml2/libxml/xmlstring.h \
 /usr/include/libxml2/libxml/xmlregexp.h \
 /usr/include/libxml2/libxml/dict.h \
 /usr/include/libxml2/libxml/xmlmemory.h \
 /usr/include/libxml2/libxml/threads.h \
 /usr/include/libxml2/libxml/globals.h \
 /usr/include/libxml2/libxml/parser.h /usr/include/libxml2/libxml/hash.h \
 /usr/include/libxml2/libxml/valid.h \
 /usr/include/libxml2/libxml/xmlerror.h \
 /usr/include/libxml2/libxml/list.h \
 /usr/include/libxml2/libxml/xmlautomata.h \
 /usr/include/libxml2/libxml/entities.h \
 /usr/include/libxml2/libxml/encoding.h \
 /usr/include/libxml2/libxml/xmlIO.h /usr/include/libxml2/libxml/SAX.h \
 /usr/include/libxml2/libxml/xlink.h /usr/include/libxml2/libxml/SAX2.h \
 ../../str.h checker.h ../../sr_module.h sip.h ../../data_lump_rpl.h \
 ../../parser/msg_parser.h ../../parser/contact/parse_contact.h \
 ../../parser/contact/../hf.h ../../parser/contact/../../str.h \
 ../../parser/contact/../msg_parser.h ../../parser/contact/contact.h \
 ../../parser/contact/../parse_param.h ../../parser/parse_uri.h \
 ../../parser/../str.h ../../parser/../parser/msg_parser.h \
 ../../parser/parse_from.h ../../parser/msg_parser.h \
 ../../parser/parse_content.h ../../parser/parse_disposition.h \
 ../../db/db.h ../../db/db_key.h ../../db/db_op.h ../../db/db_val.h \
 ../../db/../str.h ../../db/db_con.h ../../db/db_row.h ../../db/db_res.h \
 ../../db/db_cap.h ../tm/tm_load.h ../tm/defs.h ../tm/../../sr_module.h \
 ../tm/t_hooks.h ../tm/uac.h ../tm/../../str.h ../tm/dlg.h \
 ../tm/../../parser/parse_rr.h ../tm/../../parser/msg_parser.h \
 ../tm/../../parser/parse_nameaddr.h ../tm/../../parser/../str.h \
 ../tm/../../parser/parse_param.h ../tm/../../parser/hf.h \
 ../tm/../../parser/msg_parser.h ../tm/h_table.h ../tm/../../types.h \
 ../tm/../../md5utils.h ../tm/../../usr_avp.h ../tm/../../timer.h \
 ../tm/../../clist.h ../tm/../../dprint.h ../tm/../../timer_ticks.h \
 ../tm/config.h ../tm/../../hash_func.h ../tm/../../str.h \
 ../tm/../../hashes.h ../tm/../../mem/shm_mem.h ../tm/lock.h \
 ../tm/../../dprint.h ../tm/../../locking.h ../tm/sip_msg.h \
 ../tm/t_reply.h ../tm/../../rpc.h ../tm/../../tags.h \
 ../tm/../../parser/msg_parser.h ../tm/../../globals.h \
 ../tm/../../types.h ../tm/../../ip_addr.h ../tm/../../poll_types.h \
 ../tm/../../crc.h ../tm/../../socket_info.h ../tm/../../dns_cache.h \
 ../tm/../../timer.h ../tm/../../atomic_ops.h \
 ../tm/../../atomic/atomic_x86.h ../tm/../../resolve.h \
 ../tm/../../dns_wrappers.h ../tm/t_fwd.h ../tm/../../proxy.h \
 ../tm/../../config.h ../tm/t_lookup.h ../tm/t_funcs.h \
 ../tm/../../globals.h ../tm/../../udp_server.h \
 ../tm/../../msg_translator.h ../tm/../../forward.h ../tm/../../route.h \
 ../tm/../../error.h ../tm/../../route_struct.h ../tm/../../str_hash.h \
 ../tm/../../mem/mem.h ../tm/../../proxy.h ../tm/../../stats.h \
 ../tm/../../udp_server.h ../tm/../../tcp_server.h ../tm/../../mem/mem.h \
 ../tm/../../ip_addr.h ../tm/../../parser/parse_uri.h ../tm/timer.h \
 ../tm/ut.h ../tm/../../ut.h ../tm/../../comp_defs.h ../tm/../../error.h \
 ../tm/../../resolve.h ../tm/t_cancel.h mark.h ../../parser/msg_parser.h \
 ../../lump_struct.h ../../data_lump.h ../../lump_struct.h \
 ../../parser/hf.h mod.h ../../dprint.h ../../mem/mem.h \
 ../scscf/scscf_load.h ../scscf/registrar_storage.h \
 ../scscf/../tm/tm_load.h ../scscf/../../qvalue.h \
 ../scscf/registrar_parser.h ../scscf/ifc_datastruct.h \
 ../scscf/registrar_notify.h ../scscf/ims_pm.h
