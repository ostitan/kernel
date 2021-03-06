/*
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* includes */

#include <nanokernel.h>
#include <nano_private.h>
#include <string.h>
#include <debug/gdb_arch.h>
#include <misc/debug/gdb_server.h>

/* defines */

#define TRACE_FLAG          (0x0100)  /* TF in EFLAGS */
#define INT_FLAG            (0x0200)  /* IF in EFLAGS */

/* statics */

/* forward declarations */

#ifdef GDB_ARCH_HAS_RUNCONTROL
static void gdb_bp_handler (NANO_ESF * pEsf);
static void gdb_trace_handler (NANO_ESF * esf);
static void gdb_fpe_handle (NANO_ESF * esf);
static void gdb_pfault_handle (NANO_ESF * esf);
#ifdef GDB_ARCH_HAS_HW_BP
static int gdb_hw_bp_find (GDB_DBG_REGS * regs, GDB_BP_TYPE * bp_type,
                            long * address);
#endif
#endif

/**
 *
 * @brief Initialize GDB server architecture part
 *
 * This routine initializes the architecture part of the GDB server. It mostly
 * register exception handler.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_init (void) {
#ifdef GDB_ARCH_HAS_RUNCONTROL
    nanoCpuExcConnect (IV_DIVIDE_ERROR, gdb_fpe_handle);
    nanoCpuExcConnect (IV_DEBUG, gdb_trace_handler);
    nanoCpuExcConnect (IV_BREAKPOINT, gdb_bp_handler);
    nanoCpuExcConnect (IV_PAGE_FAULT, gdb_pfault_handle);
#endif
}

/**
 *
 * @brief Fill a GDB register set from a given ESF register set
 *
 * This routine fills the provided GDB register set with values from given
 * NANO_ESF register set.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_from_esf (GDB_REG_SET * regs, NANO_ESF *esf) {
	    regs->regs.eax = esf->eax;
	    regs->regs.ecx = esf->ecx;
	    regs->regs.edx = esf->edx;
	    regs->regs.ebx = esf->ebx;
	    regs->regs.esp = esf->esp;
	    regs->regs.ebp = esf->ebp;
	    regs->regs.esi = esf->esi;
	    regs->regs.edi = esf->edi;
	    regs->regs.eip = esf->eip;
	    regs->regs.eflags = esf->eflags;
	    regs->regs.cs = esf->cs;
}

/**
 *
 * @brief Fill a GDB register set from a given ISF register set
 *
 * This routine fills the provided GDB register set with values from given
 * NANO_ISF register set.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_from_isf (GDB_REG_SET *regs, NANO_ISF *isf) {
    memcpy(&regs->regs, isf, sizeof(regs->regs));
}

/**
 *
 * @brief Fill an ESF register set from a given GDB register set
 *
 * This routine fills the provided NANO_ESF register set with values
 * from given GDB register set.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_to_esf (GDB_REG_SET * regs, NANO_ESF *esf) {
	    esf->eax = regs->regs.eax;
	    esf->ecx = regs->regs.ecx;
	    esf->edx = regs->regs.edx;
	    esf->ebx = regs->regs.ebx;
	    esf->esp = regs->regs.esp;
	    esf->ebp = regs->regs.ebp;
	    esf->esi = regs->regs.esi;
	    esf->edi = regs->regs.edi;
	    esf->eip = regs->regs.eip;
	    esf->eflags = regs->regs.eflags;
	    esf->cs = regs->regs.cs;
}

/**
 *
 * @brief Fill an ISF register set from a given GDB register set
 *
 * This routine fills the provided NANO_ISF register set with values
 * from given GDB register set.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_to_isf (GDB_REG_SET * regs, NANO_ISF *isf) {
    memcpy(isf, &regs->regs, sizeof(NANO_ISF));
}

/**
 *
 * @brief Fill given buffer from given register set
 *
 * This routine fills the provided buffer with values from given register set.
 * The provided buffer must be large enough to store all register values.
 * It is up to the caller to do this check.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_get (GDB_REG_SET * regs, char * buffer) {
    *((uint32_t *)buffer) = regs->regs.eax;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.ecx;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.edx;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.ebx;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.esp;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.ebp;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.esi;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.edi;
    buffer += 4;
    *((uint32_t *)buffer) = (uint32_t) regs->regs.eip;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.eflags;
    buffer += 4;
    *((uint32_t *)buffer) = regs->regs.cs;
}

/**
 *
 * @brief Write given registers buffer to GDB register set
 *
 * This routine fills given register set with value from provided buffer.
 * The provided buffer must be large enough to contain all register values.
 * It is up to the caller to do this check.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_regs_set (GDB_REG_SET * regs,  char * buffer) {
    regs->regs.eax = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.ecx = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.edx = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.ebx = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.esp = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.ebp = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.esi = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.edi = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.eip = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.eflags = *((uint32_t *)buffer);
    buffer += 4;
    regs->regs.cs = *((uint32_t *)buffer);
}

/**
 *
 * @brief Get size and offset of given register
 *
 * This routine returns size and offset of given register.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_arch_reg_info_get (int reg_id, int * size, int * offset) {
    /* Determine register size */
    if (reg_id >= 0 && reg_id < GDBNUMREGS)
        *size = 4;

    /* Determine register offset */
    if (reg_id >= 0 && reg_id < GDBNUMREGS)
        *offset = 4 * reg_id;
}

#ifdef GDB_ARCH_HAS_RUNCONTROL
#ifdef GDB_ARCH_HAS_HW_BP
/**
 *
 * @brief Get the HW BP architecture type for a common GDB type
 *
 * This routine gets the specific architecture value that corresponds to a
 * common hardware breakpoint type.
 *
 * @return The architecture type
 *
 * \NOMANUAL
 */

static char gdb_hw_bp_type_get (GDB_BP_TYPE type, int length, GDB_ERROR_CODE * err) {
    char hw_type = -1;

    switch (type) {
        /* Following combinations are supported on IA */
        case GDB_HW_INST_BP:
            hw_type = 0;
            break;
        case GDB_HW_DATA_WRITE_BP:
            if (length == 1) {
                hw_type = 0x1;
            } else if (length == 2) {
                hw_type = 0x5;
            } else if (length == 4) {
                hw_type = 0xd;
            } else if (length == 8) {
                hw_type = 0x9;
            }
            break;
        case GDB_HW_DATA_ACCESS_BP:
            if (length == 1) {
                hw_type = 0x3;
            } else if (length == 2) {
                hw_type = 0x7;
            } else if (length == 4) {
                hw_type = 0xf;
            } else if (length == 8) {
                hw_type = 0xb;
            }
            break;
        case GDB_HW_DATA_READ_BP:
            /* Data read not supported on IA */
            /*
             * NOTE: Read only watchpoints are not supported by IA debug
             * registers, but it could be possible to use RW watchpoints
             * and ignore the RW watchpoint if it has been hit by a write
             * operation.
             */
            *err = GDB_ERROR_HW_BP_NOT_SUP;
            return -1;
        default:
            /* Unknown type */
            *err = GDB_ERROR_HW_BP_INVALID_TYPE;
            return -1;
    }
    return hw_type;
}

/**
 *
 * @brief Set the debug registers for a specific HW BP.
 *
 * This routine sets the <regs> debug registers according to the HW breakpoint
 * description.
 *
 * @return 0 debug registers have been modified or -1 on error
 *
 * \NOMANUAL
 */

int gdb_hw_bp_set (GDB_DBG_REGS * regs, long addr, GDB_BP_TYPE type, int length, GDB_ERROR_CODE * err) {
    char hw_type;

    if ((hw_type = gdb_hw_bp_type_get (type, length, err)) < 0) {
        return -1;
    }

    if (regs->db0 == 0) {
        regs->db0 = addr;
        regs->db7 |= (hw_type << 16) | 0x02;
    }
    else if (regs->db1 == 0) {
        regs->db1 = addr;
        regs->db7 |= (hw_type << 20) | 0x08;
    }
    else if (regs->db2 == 0) {
        regs->db2 = addr;
        regs->db7 |= (hw_type << 24) | 0x20;
    }
    else if (regs->db3 == 0) {
        regs->db3 = addr;
        regs->db7 |= (hw_type << 28) | 0x80;
    }
    else {
        *err = GDB_ERROR_HW_BP_DBG_REGS_FULL;
        return -1;
    }

    /* set GE bit if it is data breakpoint */
    if (hw_type != 0) {
        regs->db7 |= 0x200;
    }
    return 0;
}

/**
 *
 * @brief Clear the debug registers for a specific HW BP.
 *
 * This routine updates the <regs> debug registers to remove a HW breakpoint.
 *
 * @return 0 debug registers have been modified or -1 on error
 *
 * \NOMANUAL
 */

int gdb_hw_bp_clear (GDB_DBG_REGS * regs, long addr, GDB_BP_TYPE type, int length, GDB_ERROR_CODE * err) {
    char hw_type;

    if ((hw_type = gdb_hw_bp_type_get (type, length, err)) < 0) {
        return -1;
    }

    if ((regs->db0 == addr) && (((regs->db7 >> 16) & 0xf) == hw_type)) {
        regs->db0 = 0;
        regs->db7 &= ~((hw_type << 16) | 0x02);
    } else if ((regs->db1 == addr) &&
                (((regs->db7 >> 20) & 0xf) == hw_type)) {
        regs->db1 = 0;
        regs->db7 &= ~((hw_type << 20) | 0x08);
    } else if ((regs->db2 == addr) &&
                (((regs->db7 >> 24) & 0xf) == hw_type)) {
        regs->db2 = 0;
        regs->db7 &= ~((hw_type << 24) | 0x20);
    } else if ((regs->db3 == addr) &&
                (((regs->db7 >> 28) & 0xf) == hw_type)) {
        regs->db3 = 0;
        regs->db7 &= ~((hw_type << 28) | 0x80);
    } else {
        /* Unknown breakpoint */
        *err = GDB_ERROR_INVALID_BP;
        return -1;
    }
    return 0;
}

/**
 *
 * @brief Look for a Hardware breakpoint
 *
 * This routine checks from the <regs> debug register set if a Hardware
 * breakpoint has been hit.
 *
 * @return 0 if a HW BP has been found, -1 otherwise
 *
 * \NOMANUAL
 */

static int gdb_hw_bp_find (GDB_DBG_REGS * regs, GDB_BP_TYPE * bp_type, long * address) {
    int ix;
    unsigned char type = 0;
    long addr = 0;
    int status_bit;
    int enable_bit;

    /* get address and type of breakpoint from DR6 and DR7 */
    for (ix = 0; ix < 4; ix++) {
        status_bit = 1 << ix;
        enable_bit = 2 << (ix << 1);

        if ((regs->db6 & status_bit) && (regs->db7 & enable_bit)) {
            switch (ix) {
                case 0:
                    addr = regs->db0;
                    type = (regs->db7 & 0x000f0000) >> 16;
                    break;

                case 1:
                    addr = regs->db1;
                    type = (regs->db7 & 0x00f00000) >> 20;
                    break;

                case 2:
                    addr = regs->db2;
                    type = (regs->db7 & 0x0f000000) >> 24;
                    break;

                case 3:
                    addr = regs->db3;
                    type = (regs->db7 & 0xf0000000) >> 28;
                    break;
            }
        }
    }

    if ((addr == 0) && (type == 0))
        return -1;

    *address = addr;
    switch (type) {
        case 0x1:
        case 0x5:
        case 0xd:
        case 0x9:
            *bp_type = GDB_HW_DATA_WRITE_BP;
            break;
        case 0x3:
        case 0x7:
        case 0xf:
        case 0xb:
            *bp_type = GDB_HW_DATA_ACCESS_BP;
            break;
        default:
	        *bp_type = GDB_HW_INST_BP;
            break;
    }
    return 0;
}

/**
 *
 * @brief Clear all debug registers.
 *
 * This routine clears all debug registers
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_dbg_regs_clear (void) {
	GDB_DBG_REGS regs;

    regs.db0 = 0;
    regs.db1 = 0;
    regs.db2 = 0;
    regs.db3 = 0;
    regs.db6 = 0;
    regs.db7 = 0;
    gdb_dbg_regs_set (&regs);
}
#endif

/**
 *
 * @brief Clear trace mode
 *
 * This routine makes CPU trace-disable.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

void gdb_trace_mode_clear (GDB_REG_SET * regs, int arg) {
	regs->regs.eflags &= ~INT_FLAG;
	regs->regs.eflags |= (arg & INT_FLAG);
	regs->regs.eflags &= ~TRACE_FLAG;
}

/**
 *
 * @brief Test if single stepping is possible for current PC
 *
 * This routine indicates if step is possible for current PC
 *
 * @return 1 if it is possible to step the instruction; -1 otherwise
 *
 * \NOMANUAL
 */

int gdb_arch_can_step (GDB_REG_SET * regs) {
    unsigned char * pc = (unsigned char *) regs->regs.eip;

    if (*pc == 0xf4) return 0;         /* hlt instruction */
    return (1);
}

/**
 *
 * @brief Set trace mode
 *
 * This routine makes CPU trace-enable.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

int gdb_trace_mode_set (GDB_REG_SET * regs) {
    int tmp;
    unsigned char * pc = (unsigned char *) regs->regs.eip;

    tmp = regs->regs.eflags;
    if (*pc == 0xfb) tmp |= INT_FLAG;   /* sti instruction */
    if (*pc == 0xfa) tmp &= ~INT_FLAG;  /* cli instruction */
    regs->regs.eflags &= ~INT_FLAG;
    regs->regs.eflags |= TRACE_FLAG;

    return (tmp);
}

/**
 *
 * @brief GDB trace handler
 *
 * The GDB trace handler is used to catch and handle the trace mode exceptions
 * (single step).
 *
 * @return N/A
 *
 * \NOMANUAL
 */

static void gdb_trace_handler (NANO_ESF * esf) {
#ifdef GDB_ARCH_HAS_HW_BP
	GDB_DBG_REGS regs;
#endif

    (void)irq_lock();
#ifndef GDB_ARCH_HAS_HW_BP
    gdb_handler (GDB_EXC_TRACE, esf, GDB_SIG_TRAP);
#else
    gdb_dbg_regs_get (&regs);
    if ((regs.db6  & 0x00004000) == 0x00004000) {
        gdb_handler (GDB_EXC_TRACE, esf, GDB_SIG_TRAP);
    } else {
        int type;
        long addr;

        gdb_dbg_regs_clear ();
        (void) gdb_hw_bp_find (&regs, &type, &addr);
        gdb_cpu_stop_hw_bp_addr = addr;
        gdb_cpu_stop_bp_type = type;
        gdb_debug_status = DEBUGGING;
        gdb_handler (GDB_EXC_BP, esf, GDB_SIG_TRAP);
    }
#endif
}

/**
 *
 * @brief GDB breakpoint handler
 *
 * The GDB breakpoint handler is used to catch and handle the breakpoint
 * exceptions.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

static void gdb_bp_handler (NANO_ESF * esf) {
    (void)irq_lock();
    gdb_debug_status = DEBUGGING;
#ifdef GDB_ARCH_HAS_HW_BP
    gdb_cpu_stop_bp_type = GDB_SOFT_BP;
#endif
    esf->eip -= sizeof(GDB_INSTR);

    gdb_handler (GDB_EXC_BP, esf, GDB_SIG_TRAP);
}

/**
 *
 * @brief GDB Floating point handler
 *
 * This GDB handler is used to catch and handle the floating point exceptions.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

static void gdb_fpe_handle (NANO_ESF * esf) {
    (void)irq_lock();
    gdb_debug_status = DEBUGGING;
    gdb_handler (GDB_EXC_OTHER, esf, GDB_SIG_FPE);
    }

/**
 *
 * @brief GDB page fault handler
 *
 * This GDB handler is used to catch and handle the page fault exceptions.
 *
 * @return N/A
 *
 * \NOMANUAL
 */

static void gdb_pfault_handle (NANO_ESF * esf) {
    (void)irq_lock();
    gdb_debug_status = DEBUGGING;
    gdb_handler (GDB_EXC_OTHER, esf, GDB_SIG_SIGSEGV);
    }
#endif
