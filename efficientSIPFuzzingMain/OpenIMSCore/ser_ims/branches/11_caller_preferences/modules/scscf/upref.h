
#ifndef _UPREF_SCSCF_MOD_H
#define _UPREF_SCSCF_MOD_H


#include <stdio.h>
#include <stdlib.h> 
#include "../../parser/contact/contact.h"
#include "../../parser/contact/parse_contact.h"
#include "registrar_storage.h"
#include "mod.h"

enum _upref_bool{FALSE, TRUE} ;
enum _upref_tag_type{UNDEF, BOOL_PREF, ENUM_PREF, NUM_PREF, STR_PREF} ;
enum _duplex_type{FULL_DPLX, HALF_DPLX, RECIEVE_ONLY, SEND_ONLY }  ;
enum _class_type{BUSINESS, PERSONAL} ;
enum _mobility_type{FIXED, MOBILE} ;
enum _method_type{ACK_PREF, BYE_PREF, CANCEL_PREF, INVITE_PREF, MESSAGE_PREF, OPTIONS_PREF};

struct _upref_enum{
    short* val_list ;
    short  val_list_len ;
};

struct _upref_str{
    short tag_class ;
    unsigned char* pref_str ;
    short pref_str_len; 
};

struct _upref_custom{

    struct _upref_str name ;
    enum _upref_tag_type ttype ; 
    union _tagcore{
    	enum _upref_bool bvalue ;
	struct _upref_enum evalue ;
   	struct _upref_str svalue ; 
    }tagcore;
    struct _upref_custom*  next ;	/* dll */ 
    struct _upref_custom*  prev ;
};

struct _upref_struct{
    str content ;		
    struct _delim_catalog* catalog;	/* marker */     
    short catalog_len ;
    double qvalue ;
    enum _upref_bool iptv  ;
    enum _upref_bool text  ;
    enum _upref_bool data  ;
    enum _upref_bool audio ;
    enum _upref_bool video ;
    enum _upref_bool control ;
    enum _upref_bool isfocus ;
    enum _upref_bool automata ;
    enum _upref_bool application;
    struct _upref_enum class_pr ; 	/* class as preference tag */ 
    struct _upref_enum methods ;
    struct _upref_enum mobility ;
    struct _upref_str description ;
    struct _upref_custom custom_pref;
};


struct _delim_catalog{
    unsigned char sc_offset ;
    unsigned char eq_offset ;
};


struct _upref_avp{
    unsigned char* catalog_name ;
    short enum_value ;
};

typedef struct sip_msg sip_msg ;
typedef struct _upref_struct upref_struct ;
typedef struct _delim_catalog delim_catalog ;
typedef struct _upref_enum upref_enum ;
typedef struct _upref_str upref_str ;
typedef struct _upref_avp upref_avp ;
typedef enum _upref_bool upref_bool ;
typedef enum _upref_tag_type upref_tag_type ;

/* user preference catalog */ 
#ifndef CLASS_NUMBER 
    #define CLASS_NUMBER  sizeof(class_list)/sizeof(short) 
#endif 

static short class_list[] = { 1, 4, 5, 7, 8, 11 } ;
static unsigned char* label_class1[] = { "q", '\0'} ;
static unsigned char* label_class4[] = { "text", "data", "iptv", '\0' } ;
static unsigned char* label_class5[] = { "audio", "video", "class", '\0'} ;
static unsigned char* label_class7[] = { "control", "isfocus", "methods", '\0' } ;
static unsigned char* label_class8[] = { "automata", "mobility", '\0'} ;
static unsigned char* label_class11[] = {"application", "description", '\0'} ;

static struct _upref_avp enum_mobility_vals[] =  {{"fixed", FIXED}, 
    						  {"mobile", MOBILE}, 
						  {NULL, UNDEF}
						 };
static struct _upref_avp enum_class_vals[] = {  {"business", BUSINESS}, 
    						{"personal", PERSONAL}, 
						{NULL, UNDEF}
					      };

static struct _upref_avp enum_methods_vals[] ={	{"ACK", ACK_PREF},
						{"BYE", BYE_PREF},
						{"CANCEL", CANCEL_PREF},
						{"INVITE", INVITE_PREF},
						{"MESSAGE", MESSAGE_PREF},
						{"OPTIONS", OPTIONS_PREF}
					      };
						
// static str* enum_label_class[]= {   {"duplex",6},
//				    {"events", 6}
//				};
// static str* int_label_class[] =	{   {"q", 1},
//    {"priority", 8} 
//				};

/* boolean media feature tag map, see  rfc 3840 */ 
enum pref_class_map_1{  QVALUE = 10 } ;
enum pref_class_map_4{  TEXT = 40 ,
			DATA = 41,
			IPTV = 42 
		     };
enum pref_class_map_5{ AUDIO = 50, 
    		       VIDEO = 51,
		       CLASS = 52,
		     };
enum pref_class_map_7{  CONTROL = 70,
    			ISFOCUS = 71,
			METHODS = 72
		     };
enum pref_class_map_8{ AUTOMATA = 80,
		       MOBILITY = 81 
		     } ;
enum pref_class_map_11{ APPLICATION = 110,
			DESCRIPTION = 111} ;

/* catalog copy */ 
#ifndef CATALOG_SHMCPY
    #define CATALOG_SHMCPY(dst_cat, dst_cat_len, src_cat, src_cat_len)	{\
    					int _byte_bulk; \
					if( (src_cat_len) == 0){ \
					    LOG(L_DBG,"DBG"M_NAME"Null length cannot be copied\n") ; \
					    (dst_cat_len) = 0 ; \
					    (dst_cat) = NULL ; \
					} else { \
					    (dst_cat_len) = (src_cat_len) ;\
					    _byte_bulk = (sizeof(struct _delim_catalog)*dst_cat_len) ;\
 					    (dst_cat) = shm_malloc(_byte_bulk) ; \
					    memset((dst_cat), 0x00, _byte_bulk);	\
					    memcpy((dst_cat), (src_cat), _byte_bulk ) ; \
					}\
}
#endif

#ifndef MAP_ELM2_ENUMTYPE
    #define MAP_ELM2_ENUMTYPE(class_no, el_no, res)		res = ( ((class_no)* 10) + (el_no));
#endif

#define MAP_CLSNO2_ARR(class_no, label_pp)	\
    switch(class_no){\
	case 4:\
	    label_pp = label_class4 ; \
	break ; \
	case 5:\
	    label_pp = label_class5 ;\
	break; \
	case 7:\
	    label_pp = label_class7 ;\
	break;\
	case 8:\
	    label_pp = label_class8 ;\
	break;\
	case 11:\
	    label_pp = label_class11 ;\
	default:\
	break;\
    }


#define PRINT_USER_PREF_CATALOG(h)	{\
	int _i; \
	for(_i = 0 ; _i < h->catalog_len ; _i++) {\
	    LOG(L_INFO, "%s:%d eq_pos:%d\n", ((_i==0)?"content_begin":"sc_pos"), h->catalog[_i].sc_offset, h->catalog[_i].eq_offset);\
	}\
}

#endif  /* _UPREF_SCSCF_MOD_H */ 
