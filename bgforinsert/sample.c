#include "postgres.h"
#include "pgstat.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/proc.h"
#include "fmgr.h"
#include "/usr/include/errno.h"
#include "/usr/include/x86_64-linux-gnu/bits/types/sig_atomic_t.h"
#include "access/xact.h"
#include "executor/spi.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"

PG_MODULE_MAGIC;

void _PG_init(void);

void sample_main(Datum main_arg) pg_attribute_noreturn();

void execute_insert();

static volatile sig_atomic_t got_sigterm = false;

static void sample_sigterm(SIGNAL_ARGS)
{
	int save_errno = errno;
	got_sigterm = true;
	if (MyProc)
		SetLatch(&MyProc->procLatch);
	errno = save_errno;
}


void sample_main(Datum main_arg){
    const char *db = "postgres";
    const char *username = "muguntha-pt5620";
	pqsignal(SIGTERM, sample_sigterm);
	BackgroundWorkerUnblockSignals();
    BackgroundWorkerInitializeConnection(db,username,0);
    while (!got_sigterm)
	{
		WaitLatch(&MyProc->procLatch,
				  WL_LATCH_SET | WL_TIMEOUT,
				  10000L,
				  PG_WAIT_EXTENSION);
        
		ResetLatch(&MyProc->procLatch);
        execute_insert();
	}
    proc_exit(0);
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
    appendStringInfo(&buf,"INSERT INTO toppers ("
    "SELECT students.name,students.student_id,score "
    "FROM students,(SELECT student_id,(english+maths+science+social+gk) as score "
    "FROM marks "
    "WHERE (english+maths+science+social+gk) in ("
    "SELECT MAX(english+maths+science+social+gk) as score from marks)) 	AS temp "
    "WHERE students.student_id=temp.student_id"
    ");");
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
	snprintf(worker.bgw_library_name, BGW_MAXLEN, "sample");
	snprintf(worker.bgw_function_name, BGW_MAXLEN, "sample_main");
	snprintf(worker.bgw_name, BGW_MAXLEN, "samplebg ");
	worker.bgw_restart_time = BGW_NEVER_RESTART;
    worker.bgw_main_arg = (Datum) 0;
    worker.bgw_notify_pid = 0;
    RegisterBackgroundWorker(&worker);
}