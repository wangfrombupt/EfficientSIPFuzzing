sip.o sip.d : sip.c ../../str.h ../../parser/parse_expires.h \
 ../../parser/../str.h ../../parser/hf.h ../../parser/../comp_defs.h \
 ../../dprint.h ../../mem/mem.h ../../mem/../config.h \
 ../../mem/../types.h ../../mem/../dprint.h ../../mem/q_malloc.h \
 ../../mem/meminfo.h ../scscf/scscf_load.h ../scscf/../../sr_module.h \
 ../scscf/../../parser/msg_parser.h ../scscf/../../parser/../comp_defs.h \
 ../scscf/../../parser/../str.h ../scscf/../../parser/../lump_struct.h \
 ../scscf/../../parser/.././parser/hf.h ../scscf/../../parser/../flags.h \
 ../scscf/../../parser/../ip_addr.h ../scscf/../../parser/../str.h \
 ../scscf/../../parser/../dprint.h ../scscf/../../parser/../md5utils.h \
 ../scscf/../../parser/../config.h ../scscf/../../parser/parse_def.h \
 ../scscf/../../parser/parse_cseq.h ../scscf/../../parser/parse_to.h \
 ../scscf/../../parser/parse_via.h ../scscf/../../parser/parse_fline.h \
 ../scscf/../../parser/hf.h ../scscf/../../parser/../error.h \
 ../scscf/../../version.h ../scscf/../../rpc.h \
 ../scscf/../../route_struct.h ../scscf/../../select.h \
 ../scscf/../../str.h ../scscf/../../usr_avp.h \
 ../scscf/registrar_storage.h ../scscf/../../locking.h \
 ../scscf/../../lock_ops.h ../scscf/../../fastlock.h \
 ../scscf/../../lock_alloc.h ../scscf/../../mem/mem.h \
 ../scscf/../../mem/shm_mem.h ../scscf/../../mem/../dprint.h \
 ../scscf/../../mem/../lock_ops.h ../scscf/../../mem/q_malloc.h \
 ../scscf/../tm/tm_load.h ../scscf/../tm/defs.h \
 ../scscf/../tm/../../sr_module.h ../scscf/../tm/t_hooks.h \
 ../scscf/../tm/uac.h ../scscf/../tm/../../str.h ../scscf/../tm/dlg.h \
 ../scscf/../tm/../../parser/parse_rr.h \
 ../scscf/../tm/../../parser/msg_parser.h \
 ../scscf/../tm/../../parser/parse_nameaddr.h \
 ../scscf/../tm/../../parser/../str.h \
 ../scscf/../tm/../../parser/parse_param.h \
 ../scscf/../tm/../../parser/hf.h \
 ../scscf/../tm/../../parser/msg_parser.h ../scscf/../tm/h_table.h \
 ../scscf/../tm/../../types.h ../scscf/../tm/../../md5utils.h \
 ../scscf/../tm/../../usr_avp.h ../scscf/../tm/../../timer.h \
 ../scscf/../tm/../../clist.h ../scscf/../tm/../../dprint.h \
 ../scscf/../tm/../../timer_ticks.h ../scscf/../tm/config.h \
 ../scscf/../tm/../../hash_func.h ../scscf/../tm/../../str.h \
 ../scscf/../tm/../../hashes.h ../scscf/../tm/../../mem/shm_mem.h \
 ../scscf/../tm/lock.h ../scscf/../tm/../../dprint.h \
 ../scscf/../tm/../../locking.h ../scscf/../tm/sip_msg.h \
 ../scscf/../tm/t_reply.h ../scscf/../tm/../../rpc.h \
 ../scscf/../tm/../../tags.h ../scscf/../tm/../../parser/msg_parser.h \
 ../scscf/../tm/../../globals.h ../scscf/../tm/../../types.h \
 ../scscf/../tm/../../ip_addr.h ../scscf/../tm/../../poll_types.h \
 ../scscf/../tm/../../crc.h ../scscf/../tm/../../socket_info.h \
 ../scscf/../tm/../../dns_cache.h ../scscf/../tm/../../timer.h \
 ../scscf/../tm/../../atomic_ops.h \
 ../scscf/../tm/../../atomic/atomic_x86.h ../scscf/../tm/../../resolve.h \
 ../scscf/../tm/../../dns_wrappers.h ../scscf/../tm/t_fwd.h \
 ../scscf/../tm/../../proxy.h ../scscf/../tm/../../config.h \
 ../scscf/../tm/t_lookup.h ../scscf/../tm/t_funcs.h \
 ../scscf/../tm/../../globals.h ../scscf/../tm/../../udp_server.h \
 ../scscf/../tm/../../msg_translator.h ../scscf/../tm/../../forward.h \
 ../scscf/../tm/../../route.h ../scscf/../tm/../../error.h \
 ../scscf/../tm/../../route_struct.h ../scscf/../tm/../../str_hash.h \
 ../scscf/../tm/../../mem/mem.h ../scscf/../tm/../../proxy.h \
 ../scscf/../tm/../../stats.h ../scscf/../tm/../../udp_server.h \
 ../scscf/../tm/../../tcp_server.h ../scscf/../tm/../../mem/mem.h \
 ../scscf/../tm/../../ip_addr.h ../scscf/../tm/../../parser/parse_uri.h \
 ../scscf/../tm/../../parser/../parser/msg_parser.h \
 ../scscf/../tm/timer.h ../scscf/../tm/ut.h ../scscf/../tm/../../ut.h \
 ../scscf/../tm/../../comp_defs.h ../scscf/../tm/../../error.h \
 ../scscf/../tm/../../resolve.h ../scscf/../tm/t_cancel.h \
 ../scscf/../../qvalue.h ../scscf/registrar_parser.h ../scscf/registrar.h \
 ../scscf/ifc_datastruct.h ../scscf/registrar_notify.h ../scscf/ims_pm.h \
 ../tm/tm_load.h sip.h ../../data_lump_rpl.h ../../parser/msg_parser.h \
 ../../parser/contact/parse_contact.h ../../parser/contact/../hf.h \
 ../../parser/contact/../../str.h ../../parser/contact/../msg_parser.h \
 ../../parser/contact/contact.h ../../parser/contact/../parse_param.h \
 ../../parser/parse_uri.h ../../parser/parse_from.h \
 ../../parser/msg_parser.h ../../parser/parse_content.h \
 ../../parser/parse_disposition.h ../../db/db.h ../../db/db_key.h \
 ../../db/db_op.h ../../db/db_val.h ../../db/../str.h ../../db/db_con.h \
 ../../db/db_row.h ../../db/db_res.h ../../db/db_cap.h mark.h \
 ../../parser/msg_parser.h ../../lump_struct.h ../../data_lump.h \
 ../../lump_struct.h ../../parser/hf.h checker.h \
 ../scscf/ifc_datastruct.h ../../sr_module.h mod.h
