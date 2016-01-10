/**
 * \brief Virtual machine integration tool guest/host common structures.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#pragma once
#ifndef __JREKCED_VMITT_H__
#define __JREKCED_VMITT_H__

/** Major version protocol. */
#define INTEGRATION_TOOL_MAJOR_VERSION 0 

/** Minor version protocol. */
#define INTEGRATION_TOOL_MINOR_VERSION 1 

/** Default I/O port number. */
#define INTEGRATION_TOOL_DEFAULT_IO_PORT 0xFFF0

/** Protocol identification. */
#define INTEGRATION_TOOL_MAGIC "VMITP" 

/** Function code : Initialisation. */
#define INTEGRATION_TOOL_FCT_INIT      0 

 /** Function code : Un-initialisation. */
#define INTEGRATION_TOOL_FCT_UNINIT  1

/** Function code : Syncrhonize host and guest. */
#define INTEGRATION_TOOL_FCT_SYNC 2 

/** Number of function into StdGuestFunctionHandles. */
#define INTEGRATION_TOOL_COUNT_STD_GUEST_FUNCTIONS (sizeof (StdGuestFunctionHandles) / sizeof(GuestHandle))

/** 16 bit function call. */
#define VM_CALL_FLAG_16BITS   0 

/** 32 bit function call. */
#define VM_CALL_FLAG_32BITS   1

/** C call convertion. */
#define VM_CALL_FLAG_C        0

/** Pascal call convertion. */
#define VM_CALL_FLAG_PASCAL   4


#pragma pack(push, 1)

#ifndef __cplusplus
	typedef
#endif

/**
 * \brief Pointer to guest function call.
 */
struct GuestHandle {

	/** \brief Guest pointer adress. */
	union Pointer {

		/** \brief word[1] = segment adress, word[1] = offset adress. */
		unsigned short word [2];

		/** \brief 32 bits value of pointer. */
		unsigned long dword;

		/** \brief Guest pointer. */
		void * pointer;
	} guestPtr;

	/**
	 * \brief Call flags (VM_CALL_FLAG_16BITS or VM_CALL_FLAG_32BITS) | (VM_CALL_FLAG_C or VM_CALL_FLAG_PASCAL).
	 */
	unsigned short flags;
};

/** List of standard guest functions handles. */
struct StdGuestFunctionHandles {

	/** \brief Guest pointer handle to ShutdownSystem function. */
	GuestHandle ShutdownSystem;

	/** \brief Guest pointer handle to SetMousePos function. */
	GuestHandle SetMousePos;
};

/** \brief Data transfert structure. */
struct DataTransfertBlock {
	
	/** \brief Function code. */
	unsigned short function;

	/** \brief Data parameters. */
	union Data {

		/** \brief Initialisation send by host to guest. */
		struct InitHost {

			/** \brief Protocol magic code = INTEGRATION_TOOL_MAGIC. */
			char magic [6];

			/** \brief Protocol major version = INTEGRATION_TOOL_MAJOR_VERSION. */
			unsigned short majorVersion;

			/** \brief Protocol minor version = INTEGRATION_TOOL_MINOR_VERSION. */
			unsigned short minorVersion;

		} initHost;

		/** \brief Initialisation send by guest to host. */
		struct InitGuest {

			/** \brief Protocol magic code = INTEGRATION_TOOL_MAGIC. */
			char magic   [6];

			/** \brief Guest OS identifivation. */
			char guestId [8];

			/** \brief Protocol major version = INTEGRATION_TOOL_MAJOR_VERSION. */
			unsigned short majorVersion;

			/** \brief Protocol linor version = INTEGRATION_TOOL_MINOR_VERSION. */
			unsigned short minorVersion;
			
			/** \brief Number of function into guestFunctionHandels ( = INTEGRATION_TOOL_COUNT_STD_GUEST_FUNCTIONS). */
			unsigned short guestFunctionHandlesCount;

			/** Standard guest function handles. */
			StdGuestFunctionHandles stdFct;

		} initGuest;
	} data;
};

#pragma pack(pop)

#endif
