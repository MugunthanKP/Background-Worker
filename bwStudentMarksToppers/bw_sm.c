#include "postgres.h"
#include "pgstat.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "/usr/include/errno.h"
#include "/usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h"
#include "access/xact.h"
#include "executor/spi.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"
#include "miscadmin.h"
#include <time.h>
#include <string.h>

PG_MODULE_MAGIC;

void bw_sm_main(Datum main_arg) pg_attribute_noreturn();

static int isExtensionReady(void);

static int check_pg_is_in_recovery(void);

static void execute_students_insert(void);

static void execute_marks_insert(void);

static volatile sig_atomic_t got_sigterm = false;

static void bw_sm_sigterm(SIGNAL_ARGS)
{
	int save_errno = errno;
	got_sigterm = true;
	if (MyProc)
		SetLatch(&MyProc->procLatch);
	errno = save_errno;
}

void bw_sm_main(Datum main_arg){
    const char *db = "postgres";
    const char *username = "muguntha-pt5620";
	pqsignal(SIGTERM, bw_sm_sigterm);
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
            execute_marks_insert();
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

static void execute_students_insert(void){

    int ret;
    char *ch ;
    StringInfoData buf;
    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot(GetTransactionSnapshot());
    pgstat_report_activity(STATE_RUNNING, "executing configuration logger function");
    for ( int i = 1 ; i <= 100 ; i++){
        ch= "INSERT INTO students VALUES (%d,'name%d','address%d'"
                    ",TO_DATE('20010101','YYYYMMDD') + (random() * (interval '365 days'))"
                    ",cast(floor(random()*(9999999999-6000000000+1)+6000000000)as varchar))";
        initStringInfo(&buf);
        appendStringInfo(&buf,ch,i,i,i);
        debug_query_string = buf.data;
        ret = SPI_execute(buf.data, false, 1);
    }
    SPI_finish();
    PopActiveSnapshot();
    CommitTransactionCommand();
    pgstat_report_activity(STATE_IDLE, 0);
}


static void execute_marks_insert(void){
    char *ch ;
    int ret;
    int student_id,english,maths,science,social,gk;
    StringInfoData buf;
    srand(time(0));
    SetCurrentStatementStartTimestamp();
    StartTransactionCommand();
    SPI_connect();
    PushActiveSnapshot(GetTransactionSnapshot());
    pgstat_report_activity(STATE_RUNNING, "executing configuration logger function");
    for ( int i = 1 ; i <= 100 ; i++){
        ch = "INSERT INTO marks VALUES (%d,%d,%d,%d,%d,%d,CURRENT_TIMESTAMP)";
        student_id = i;
        english = (rand() % (100 - 60 + 1)) + 60;
        maths = (rand() % (100 - 60 + 1)) + 60;
        science = (rand() % (100 - 60 + 1)) + 60;
        social = (rand() % (100 - 60 + 1)) + 60;
        gk = (rand() % (100 - 60 + 1)) + 60;
        initStringInfo(&buf);
        appendStringInfo(&buf,ch,student_id,english,maths,science,gk,social);
        debug_query_string = buf.data;
        ret = SPI_execute(buf.data, false, 1);
    }
    SPI_finish();
    PopActiveSnapshot();
    CommitTransactionCommand();
    pgstat_report_activity(STATE_IDLE, 0);
}

void _PG_init(void){
    pid_t		pid;
    BackgroundWorker worker;
    MemSet(&worker,0,sizeof(BackgroundWorker));
    worker.bgw_flags = BGWORKER_SHMEM_ACCESS | BGWORKER_BACKEND_DATABASE_CONNECTION;
    worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	snprintf(worker.bgw_library_name, BGW_MAXLEN, "bw_sm");
	snprintf(worker.bgw_function_name, BGW_MAXLEN, "bw_sm_main");
	snprintf(worker.bgw_name, BGW_MAXLEN, "bw_sm ");
	worker.bgw_restart_time = BGW_NEVER_RESTART;
    worker.bgw_main_arg = (Datum) 0;
    worker.bgw_notify_pid = 0;
    RegisterBackgroundWorker(&worker);
}