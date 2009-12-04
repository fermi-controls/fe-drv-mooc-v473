// $Id$

#include "v473.h"
#include <errlogLib-2.0.h>
#include <stdexcept>
#include <cstdio>
#include <vme.h>
#include <sysLib.h>
#include <intLib.h>
#include <taskLib.h>

extern "C" UINT16 sysIn16(UINT16*);
extern "C" void sysOut16(UINT16*, UINT16);

static void init() __attribute__((constructor));
static void term() __attribute__((destructor));

static HLOG hLog = 0;

// Constructor function sets up the logger handle.

static void init()
{
    hLog = logRegister("V473", 0);
}

static void term()
{
    logUnregister(&hLog);
}

using namespace V473;

Card::Card(uint8_t addr, uint8_t)
{
    char* baseAddr;

    if (ERROR == sysBusToLocalAdrs(VME_AM_STD_SUP_DATA,
				   reinterpret_cast<char*>((uint32_t) addr << 16),
				   &baseAddr))
	throw std::runtime_error("illegal A24 VME address");

    // Compute the memory-mapped registers based upon the computed
    // base address.

    dataBuffer = reinterpret_cast<__typeof dataBuffer>(baseAddr);
    mailbox = reinterpret_cast<uint16_t*>(baseAddr + 0x7ffa);
    count = reinterpret_cast<uint16_t*>(baseAddr + 0x7ffc);
    readWrite = reinterpret_cast<uint16_t*>(baseAddr + 0x7ffe);
    reset = reinterpret_cast<uint16_t*>(baseAddr + 0xfffe);
    irqEnable = reinterpret_cast<uint16_t*>(baseAddr + 0x8000);
    irqSource = reinterpret_cast<uint16_t*>(baseAddr + 0x8002);
    irqMask = reinterpret_cast<uint16_t*>(baseAddr + 0x8004);
    irqStatus = reinterpret_cast<uint16_t*>(baseAddr + 0x8006);

    // Now that we think we're configured, let's check to see if we
    // are, indeed, a V473.

    sysOut16(mailbox, 0xff00);
    sysOut16(count, 3);
    sysOut16(readWrite, 0);
    taskDelay(2);

    if (sysIn16(readWrite) != 2 || sysIn16(count) != 3 || sysIn16(dataBuffer) != 473)
	throw std::runtime_error("VME A16 address doesn't refer to V473 hardware");

    logInform3(hLog, "V473: Found hardware -- addr %p, Firmware v%04x, FPGA v%04x",
	       dataBuffer, sysIn16(dataBuffer + 1), sysIn16(dataBuffer + 2));

    // XXX: Now that we know we're a V473, we can attach the interrupt
    // handler.
}

Card::~Card()
{
    // XXX: Shut down interrupts and shut off the hardware.
}

// Sends the mailbox value, the word count and the READ command to the
// hardware. This function assumes the data buffer has been preloaded
// with the appropriate data. Returns true if everything is
// successful.

bool Card::readProperty(vwpp::Lock const&, uint16_t mb, size_t n)
{
    sysOut16(mailbox, mb);
    sysOut16(count, (uint16_t) n);
    sysOut16(readWrite, 0);

    // Wait up to 20 milliseconds for a response.

    return intDone.wait(20);
}

// Sends the mailbox value, the word count and the SET command to the
// hardware. When this function returns, the data buffer will hold the
// return value.

bool Card::setProperty(vwpp::Lock const&, uint16_t mb, size_t n)
{
    sysOut16(mailbox, mb);
    sysOut16(count, (uint16_t) n);
    sysOut16(readWrite, 1);

    // Wait up to 20 milliseconds for a response.

    return intDone.wait(20);
}
