#include "../common/common.h"
#include "../common/tools.h"
#include "../common/savelog.h"

int main()
{
    flushlog();
    record_err(1,timeout);
    record_repeat(1,7,cksum_err);
}