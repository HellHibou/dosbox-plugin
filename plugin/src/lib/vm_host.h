/**
 * \file vm_host.h
 * \brief Virtual machine structures and definition.
 * \author Jeremy Decker
 * \version 0.1
 * \date 14/11/2015
 */
#pragma once

#define VMHOST_LOG_ERROR    1 /**< Error message level. */
#define VMHOST_LOG_WARNING  2 /**< Warning message level.*/
#define VMHOST_LOG_INFO     3 /**< Information message level.*/
#define VMHOST_LOG_DEBUG    4 /**< Debug message level.*/

#define VM_ERROR_LIBRARY_NOT_FOUND       1 /**< Library not found. */
#define VM_ERROR_UNSUPPORTED_LIBRARY     2 /**< Unsupported library. */
#define VM_ERROR_LIBRARY				 3 /**< Bad library. */

#define VM_NO_ERROR				           0 /**< No error. */
#define VM_ERROR_NULL_POINTER_EXCEPTION   -1 /**< Null pointer exception. */
#define VM_ERROR_BAD_STRUCT_SIZE          -2 /**< Bad strcuture size. */
#define VM_ERROR_BAD_PARAMETER_VALUE      -3 /**< Bad parameter value. */
#define VM_ERROR_UNSUPPORTED_VM_NAME      -4 /**< Unsupported virtual machine. */
#define VM_ERROR_UNSUPPORTED_VM_VERSION   -5 /**< Unsupported virtual machine version. */
#define VM_ERROR_UNSUPPORTED_OPERATION    -6 /**< Unsupported operation. */
#define VM_ERROR_UNSUPPORTED_COMMAND      -7 /**< Unsupported command. */
#define VM_ERROR_COMMAND_FAILED           -8 /**< Command failed. */
#define VM_ERROR_OVERFLOW                 -9 /**< Overflow. */
#define VM_UNKNOWN_ERROR                -999 /**< Unknow error. */
#define VM_CUSTOM_VM_ERROR			   -1000 /**< First specific error code of virtual machine. */

/** 16 bit function call. */
#define VM_CALL_FLAG_16BITS   0 

/** 32 bit function call. */
#define VM_CALL_FLAG_32BITS   1

/** C call convertion. */
#define VM_CALL_FLAG_C        0

/** Pascal call convertion. */
#define VM_CALL_FLAG_PASCAL   4

 /** Number of functions'spointere into VirtualMachine structure. */
#define VM_VirtualMachine_FCT_COUNT ((sizeof (vm::type::VirtualMachine) - sizeof(int)) / sizeof(void*))

/** Minimal size of VirtualMachine strcuture. */
#define VM_VirtualMachine_MINSIZE (sizeof(int) + sizeof(void*)) 

/** Size of VirtualMachineInfo.name */
#define VM_SIZEOF_VMNAME 16

#ifdef __cplusplus
	namespace vm { namespace type {
#else
	#define struct typedef struct
#endif

/**
 * \brief CPU 32 bits register.
 */
typedef union Reg32 {
	unsigned long  dword;   /**< 32 bits register value. */
	unsigned short word[2]; /**< 16 bits register value. */
	unsigned char  byte[4]; /**< 8 bits register value. */
} Reg32;

/**
* \brief Intel CPU registers.
*/
struct Regs {
	Reg32 eax; /**< EAX CPU register. */
	Reg32 ecx; /**< ECX CPU register. */
	Reg32 edx; /**< EDX CPU register. */
	Reg32 ebx; /**< EBX CPU register. */
	Reg32 esp; /**< ESP CPU register. */
	Reg32 ebp; /**< EBP CPU register. */
	Reg32 esi; /**< ESI CPU register. */
	Reg32 edi; /**< EDI CPU register. */
	Reg32 eip; /**< EIP CPU register. */
	unsigned char flags; /**< CPU flag register. */
};
	
/**
 * \brief CPU interrupt handle.
 * \return 0 --> None ; 1--> Stop
 */
typedef unsigned int (* InterruptHandle)();	

/**
 * \brief Mouse moved event hanlde.
 * \param x New X position of mouse pointer.
 * \param y New Y position of mouse pointer.
 */
typedef void (* MouseMoveEventHandle)(int x, int y);

/**
 * \brief Port writer handle.
 * \param port Port number.
 * \param val Value to write.
 * \param len Data size to write in bytes (1, 2 or 4).
 */
typedef void (* IoOutputHandle)(unsigned int port, unsigned int val, unsigned int len);

/**
 * \brief Port reader handle.
 * \param port Port to read.
 * \param len Data size to read in bytes (1, 2 or 4).
 */
typedef unsigned int (* IoInputHandle)(unsigned int port, unsigned int len);

/**
 * \struct VirtualMachineInfo
 * \brief Virtual machine information structure.
 */
struct VirtualMachineInfo
{
	int    structSize;		       /**< = sizeof(VirtualMachineInfo). */
	int    vm_version_major;       /**< Virtual machine major version number. */
	int    vm_version_minor;       /**< Virtual machine minor version number. = Number of functions into VirtualMachine's structure. */
	char   name[VM_SIZEOF_VMNAME]; /**< Virtual machine name. */
}; 

struct VirtualMachine
{
	int  structSize; /**< = sizeof(vm::type::VirtualMachine). */

    /**
	 * \brief Get virtual machine informations.
	 * \return Virtual machine informations
     */
	const VirtualMachineInfo * (*getVmInfo) ();

    /**
	 * \brief Set title of virtual machine's window.
     * \param title Title to set, NULL or epmty value for using  virtual machin's default title.
	 * \return Error code or VM_NO_ERROR if no error.
     */
	int (*setWindowTitle) (const char * title);

    /**
	 * \brief Set icon of virtual machine's window.
     * \param icon Icon's data in RGB or RGBA(if bits = 32) format. If NULL, set default icon. 
     * \param width Icon width.
     * \param height Icon height.
     * \param bits Size in bits of 1 pixels (can be 24 or 32).
	 * \return Error code or VM_NO_ERROR if no error.
     */
	int (*setWindowIcon) (const unsigned char * icon, int width, int height, int bits);

    /**
     * \brief Send a command to virtual machine.
     * \param args Command and arguments to send to virtual machine.
     * \return Error code or VM_NO_ERROR if no error.
     */
	int (*sendCommand) (const char * args, ...);

	/**
     * \brief Send a log message to virtual machine.
     * \param messageType Message level.
     * \param message Message to log.
     * \return Error code or VM_NO_ERROR if no error.
	 */
	int (*logMessage) (int messageType, const char * message);

    /**
     * \brief Get a parameter for a plugin.
     * \param parameter Name of parameter.
     * \return Parameter's value or NULL;
     */
	const char * (*getParameter) (const char * parameter);

	/**
	 * \brief Set interrupt handle.
	 * \param intId Interrupt number.
	 * \param intHnd Interrupt handle.
	 * \return Error code or VM_NO_ERROR if no error.
	 */
    int (*setInterruptHandle)(unsigned char intId, InterruptHandle intHnd);

	/**
	 * \brief Set mouse move handle.
	 * \param mHnd Mouse move handle.
	 * \return Preview mouse handle (can be null). 
	 */
	const MouseMoveEventHandle (*setMouseMoveEventHandle) (MouseMoveEventHandle mHnd);

	/**
	 * \brief Set port output handle.
	 * \param port First port number to set.
	 * \param pHnd Port output handle. 
	 * \param len Number of port to set (1, 2 or 4).
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	int (*setIoOutputHandle) (unsigned short port, IoOutputHandle pHnd, unsigned char len);

	/**
	 * \brief Get port output handle.
	 * \param port Port number.
	 * \return Port output handle.
	 */
	const IoOutputHandle (*getIoOutputHandle) (unsigned short port);

	/**
	 * \brief Set port input handle.
	 * \param port First port number to set.
	 * \param pHnd Port input handle or null if port > 65536, len <> 1, 2, 4 or pHnd = NULL.
	 * \param len Number of port to set (1, 2 or 4).
	 * \return Error code or VM_NO_ERROR if no error.
	 */
	int (*setIoInputHandle) (unsigned short port, IoInputHandle pHnd, unsigned char len);

	/**
	 * \brief Get port input handle.
	 * \param port Port number.
	 * \return Port input handle.
	 */
	const IoInputHandle (*getIoInputHandle) (unsigned short port);

	/**
     * \brief Call guest function.
     * \param segment Segment adrress of guest function to call.
     * \param offset Offset adrress of guest function to call.
     *
	 * \param callFlags Flags to define call parameters :
	 *                  VM_CALL_FLAGS_16BITS : 16 bits call.
	 *                  VM_CALL_FLAGS_32BITS : 32 bits call.
     *
     * \param stackCallArgc Number of argument to send to called function.
	 * \param stackCallArgs Arguments list to send to called function.
     * \param completeHandle Handle to function to call when execution completed.
     */
	int (*callGuestFct)(unsigned int segment, unsigned int offset, unsigned int callTypeFlags, short stackCallArgc, unsigned short * stackCallArgs);
};
#ifdef __cplusplus
	}

	/**
	 * \brief Get registers values fucntions.
	 */
	namespace reg {
#endif
		inline unsigned long EAX(vm::type::Regs reg) { return reg.eax.dword;   }
		inline unsigned short AX(vm::type::Regs reg) { return reg.eax.word[0]; }
		inline unsigned char  AL(vm::type::Regs reg) { return reg.eax.byte[0]; }
		inline unsigned char  AH(vm::type::Regs reg) { return reg.eax.byte[1]; }

		inline unsigned long EBX(vm::type::Regs reg) { return reg.ebx.dword;   }
		inline unsigned short BX(vm::type::Regs reg) { return reg.ebx.word[0]; }
		inline unsigned char  BL(vm::type::Regs reg) { return reg.ebx.byte[0]; }
		inline unsigned char  BH(vm::type::Regs reg) { return reg.ebx.byte[1]; }

		inline unsigned long ECX(vm::type::Regs reg) { return reg.ecx.dword;   }
		inline unsigned short CX(vm::type::Regs reg) { return reg.ecx.word[0]; }
		inline unsigned char  CL(vm::type::Regs reg) { return reg.ecx.byte[0]; }
		inline unsigned char  CH(vm::type::Regs reg) { return reg.ecx.byte[1]; }

		inline unsigned long EDX(vm::type::Regs reg) { return reg.edx.dword;   }
		inline unsigned short DX(vm::type::Regs reg) { return reg.edx.word[0]; }
		inline unsigned char  DL(vm::type::Regs reg) { return reg.edx.byte[0]; }
		inline unsigned char  DH(vm::type::Regs reg) { return reg.edx.byte[1]; }

		inline unsigned long ESP(vm::type::Regs reg) { return reg.esp.dword;   }
		inline unsigned long EBP(vm::type::Regs reg) { return reg.ebp.dword;   }
		inline unsigned long ESI(vm::type::Regs reg) { return reg.esi.dword;   }
		inline unsigned long EDI(vm::type::Regs reg) { return reg.edi.dword;   }
		inline unsigned long EIP(vm::type::Regs reg) { return reg.eip.dword;   }

		inline unsigned short SP(vm::type::Regs reg) { return reg.esp.word[0]; }
		inline unsigned short BP(vm::type::Regs reg) { return reg.ebp.word[0]; }
		inline unsigned short SI(vm::type::Regs reg) { return reg.esi.word[0]; }
		inline unsigned short DI(vm::type::Regs reg) { return reg.edi.word[0]; }
		inline unsigned short IP(vm::type::Regs reg) { return reg.eip.word[0]; }


		inline unsigned long EAX(vm::type::Regs *reg) { return reg->eax.dword;   }
		inline unsigned short AX(vm::type::Regs *reg) { return reg->eax.word[0]; }
		inline unsigned char  AL(vm::type::Regs *reg) { return reg->eax.byte[0]; }
		inline unsigned char  AH(vm::type::Regs *reg) { return reg->eax.byte[1]; }

		inline unsigned long EBX(vm::type::Regs *reg) { return reg->ebx.dword;   }
		inline unsigned short BX(vm::type::Regs *reg) { return reg->ebx.word[0]; }
		inline unsigned char  BL(vm::type::Regs *reg) { return reg->ebx.byte[0]; }
		inline unsigned char  BH(vm::type::Regs *reg) { return reg->ebx.byte[1]; }

		inline unsigned long ECX(vm::type::Regs *reg) { return reg->ecx.dword;   }
		inline unsigned short CX(vm::type::Regs *reg) { return reg->ecx.word[0]; }
		inline unsigned char  CL(vm::type::Regs *reg) { return reg->ecx.byte[0]; }
		inline unsigned char  CH(vm::type::Regs *reg) { return reg->ecx.byte[1]; }

		inline unsigned long EDX(vm::type::Regs *reg) { return reg->edx.dword;   }
		inline unsigned short DX(vm::type::Regs *reg) { return reg->edx.word[0]; }
		inline unsigned char  DL(vm::type::Regs *reg) { return reg->edx.byte[0]; }
		inline unsigned char  DH(vm::type::Regs *reg) { return reg->edx.byte[1]; }

		inline unsigned long ESP(vm::type::Regs *reg) { return reg->esp.dword;   }
		inline unsigned long EBP(vm::type::Regs *reg) { return reg->ebp.dword;   }
		inline unsigned long ESI(vm::type::Regs *reg) { return reg->esi.dword;   }
		inline unsigned long EDI(vm::type::Regs *reg) { return reg->edi.dword;   }
		inline unsigned long EIP(vm::type::Regs *reg) { return reg->eip.dword;   }

		inline unsigned short SP(vm::type::Regs *reg) { return reg->esp.word[0]; }
		inline unsigned short BP(vm::type::Regs *reg) { return reg->ebp.word[0]; }
		inline unsigned short SI(vm::type::Regs *reg) { return reg->esi.word[0]; }
		inline unsigned short DI(vm::type::Regs *reg) { return reg->edi.word[0]; }
		inline unsigned short IP(vm::type::Regs *reg) { return reg->eip.word[0]; }
#ifdef __cplusplus
	}
}
	#undef struct
#endif
