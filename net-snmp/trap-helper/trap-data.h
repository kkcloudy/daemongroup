/* trap-data.h */

#ifndef TRAP_DATA_H
#define TRAP_DATA_H

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct TrapParam_t {
	char *oid;
	char type;
	char *value;
} TrapParam;

typedef struct TrapData_t {
	char *oid;
	TrapList paramList;
} TrapData;

char *spaces_turn_underline(char *lte_switch_data);

TrapData *trap_data_new(const char *full_oid);

TrapData *trap_data_new_from_descr(TrapDescr *tDescr);

TrapParam *trap_param_new(const char *full_oid,
								char type,
								const char *value);

void trap_param_free(TrapParam *tParam);

void trap_data_append_param(TrapData *tData,
								const char *full_oid,
								char type,
								const char *value);

void trap_data_append_param_str(TrapData *tData,
								const char *format, ...);

void trap_data_append_common_param(TrapData *tData, TrapDescr *tDescr);

void trap_data_destroy(TrapData *tData);

void trap_send(TrapList *tRcvList, TrapList *tV3UserList, TrapData *tData);

void trap_data_test(void);

#ifdef  __cplusplus
}
#endif

#endif /* TRAP_DATA_H */

