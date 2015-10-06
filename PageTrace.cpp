#include <iostream>
#include <fstream>
#include "pin.H"

KNOB<std::string> knob_outfile(KNOB_MODE_WRITEONCE, "pintool",
        "o", "/tmp/pagetrace.out", "specify output file name");

std::ofstream outfile;
const int page_order = 12;
const int page_mask  = ~((1L << page_order)-1);

void readwrite(THREADID tid, uintptr_t ip, uintptr_t addr)
{
    outfile << tid << " " << (void*)ip
        << " " << (void*)(addr & page_mask) << std::endl;
}

void instruction(INS ins, void *v)
{
    int ops = INS_MemoryOperandCount(ins);
    for (int i = 0; i < ops; i++)
        if (INS_MemoryOperandIsRead(ins, i)
                || INS_MemoryOperandIsWritten(ins,i))
            INS_InsertPredicatedCall(ins, IPOINT_BEFORE,
                    (AFUNPTR)readwrite,
                    IARG_THREAD_ID,
                    IARG_INST_PTR,
                    IARG_MEMORYOP_EA, i,
                    IARG_END);
}

void fini(int code, void *v)
{
    outfile.close();
}

int Usage(void)
{
    std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;
    return 1;
}

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    outfile.open(knob_outfile.Value().c_str());
    outfile.setf(ios::showbase);

    INS_AddInstrumentFunction(instruction, 0);
    PIN_AddFiniFunction(fini, 0);

    PIN_StartProgram();

    return 0;
}
