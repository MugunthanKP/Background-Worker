#include "postgres.h"
#include "pgstat.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/proc.h"
#include "storage/shm_toc.h"
#include "storage/shm_mq.h"
#include "fmgr.h"
#include "/usr/include/errno.h"
#include "access/xact.h"
#include "executor/spi.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"
#include "miscadmin.h"


PG_MODULE_MAGIC;

void _PG_init(void);

void bw_topper_main(Datum main_arg) pg_attribute_noreturn();

void execute_insert(void);

static int isExtensionReady(void);

static int check_pg_is_in_recovery(void);

static volatile sig_atomic_t got_sigterm = false;

static void bw_topper_sigterm(SIGNAL_ARGS)
{
	int save_errno = errno;
	got_sigterm = true;
	if (MyProc)
		SetLatch(&MyProc->procLatch);
	errno = save_errno;
}


void bw_topper_main(Datum main_arg){
    const char *db = "postgres";
    const char *username = "muguntha-pt5620";
	pqsignal(SIGTERM, bw_topper_sigterm);
	BackgroundWorkerUnblockSignals();
    BackgroundWorkerInitializeConnection(db,username,0);
    while (!got_sigterm)
	{   
		WaitLatch(&MyProc->procLatch,
				  WL_LATCH_SET | WL_TIMEOUT,
				  10000L,
				  PG_WAIT_EXTENSION);
        
		ResetLatch(&MyProc->procLatch);
        if (isExtensionReady() && !check_pg_is_in_recovery()){
            execute_insert();
        }
	}
    proc_exit(0);
}

static int isExtensionReady(void){
    int ntup;
    int ret;
    char *ch;
    bool isnull;
    StringInfoData buf;
    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot(GetTransactionSnapshot());
    pgstat_report_activity(STATE_RUNNING, "checking extension available");
    ch = "select count(*) from pg_extension where extname='bwstudentsmarkstoppers';";
    initStringInfo(&buf);
    appendStringInfo(&buf,ch,NULL);
    debug_query_string = buf.data;
    SPI_execute(buf.data, true, 1);
    ret = SPI_getbinval(SPI_tuptable->vals[0],SPI_tuptable->tupdesc,1,&isnull);
    SPI_finish();
    PopActiveSnapshot();
    CommitTransactionCommand();
    pgstat_report_activity(STATE_IDLE, 0);
    return ret!=0;
}

static int check_pg_is_in_recovery(void){
    int ntup;
    int ret;
    char *ch;
    bool isnull;
    StringInfoData buf;
    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot(GetTransactionSnapshot());
    pgstat_report_activity(STATE_RUNNING, "checking pg is in recovery");
    ch = "SELECT pg_is_in_recovery()";
    initStringInfo(&buf);
    appendStringInfo(&buf,ch,NULL);
    debug_query_string = buf.data;
    ret = SPI_execute(buf.data, false, 1);
    ret = SPI_getbinval(SPI_tuptable->vals[0],SPI_tuptable->tupdesc,1,&isnull);
    SPI_finish();
    PopActiveSnapshot();
    CommitTransactionCommand();
    pgstat_report_activity(STATE_IDLE, 0);
    return ret;
}

void execute_insert(){
    int ret;
    StringInfoData buf;
    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot(GetTransactionSnapshot());
    pgstat_report_activity(STATE_RUNNING, "executing configuration logger function");
    initStringInfo(&buf);
    appendStringInfo(&buf,"INSERT INTO toppers(SELECT students.name,students.student_id,score,temp.time from students," 
						"(SELECT student_id,(english+maths+science+social+gk) as score,time FROM marks " 
						"ORDER BY time DESC,(english+maths+science+social+gk) DESC LIMIT 3) AS Temp " 
					"WHERE students.student_id = temp.student_id) ORDER BY score DESC;");
    debug_query_string = buf.data;
    ret = SPI_execute(buf.data, false, 1);
    SPI_finish();
    PopActiveSnapshot();
    CommitTransactionCommand();
    pgstat_report_activity(STATE_IDLE, 0);
}


void _PG_init(void){
    BackgroundWorker worker;
    MemSet(&worker,0,sizeof(BackgroundWorker));
    worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
    worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	snprintf(worker.bgw_library_name, BGW_MAXLEN, "bw_topper");
	snprintf(worker.bgw_function_name, BGW_MAXLEN, "bw_topper_main");
	snprintf(worker.bgw_name, BGW_MAXLEN, "bw_topper ");
	worker.bgw_restart_time = BGW_NEVER_RESTART;
    worker.bgw_main_arg = (Datum) 0;
    worker.bgw_notify_pid = 0;
    RegisterBackgroundWorker(&worker);
}