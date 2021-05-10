
#include "./test.h"
#include "api/api.h"
#include "base/log.h"
#include "decl/config.h"
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

status_t test_t::onStartup(std::string mark)
{
    return status_t::null;
}

//-----------------------------------------------------------------------------
status_t test_t::onHandler(channel_t::data_t& data)
{
    //logDebug("onHandler type:{}, content:{}", data->type(), data->data());

    return status_t::unknow;
}
