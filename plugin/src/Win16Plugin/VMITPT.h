/**
 * \file VMIPT.h
 * \brief Virtual machine integration tool guest/host common structures.
 * \author Jeremy Decker
 * \version 0.1
 * \date 09/01/2016
 */
#pragma once

/** Major version protocol. */
#define INTEGRATION_TOOL_MAJOR_VERSION 0 

/** Minor version protocol. */
#define INTEGRATION_TOOL_MINOR_VERSION 1 

/** I/O port number. */
#define INTEGRATION_TOOL_IO_PORT 0xFF01 

/** Protocol identification. */
#define INTEGRATION_TOOL_MAGIC "VMITP" 

/** Function code : Initialisation. */
#define INTEGRATION_TOOL_FCT_INIT      0 

 /** Function code : Un-initialisation. */
#define INTEGRATION_TOOL_FCT_UNINIT    1

/** Function code : Called by guest timer. */
#define INTEGRATION_TOOL_FCT_TIMER     2 

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

		/** \brief Guest pointer. */
		void *         guestPtr;
	} pointer;

	/**
	 * \brief Call flags (VM_CALL_FLAG_16BITS or VM_CALL_FLAG_32BITS) | (VM_CALL_FLAG_C or VM_CALL_FLAG_PASCAL).
	 */
	unsigned short flags;
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
			char magic   [6];

			/** \brief Protocol major version = INTEGRATION_TOOL_MAJOR_VERSION. */
			unsigned short majorVersion;

			/** \brief Protocol linor version = INTEGRATION_TOOL_MINOR_VERSION. */
			unsigned short minorVersion;

			/** \brief Mouse parameter to set (mouse speed, ...). */
			unsigned short systemMouseInfo[3];

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
			
			/** \brief Guest pointer handle to ShutdownSystem function. */
			GuestHandle ShutdownSystem;

			/** \brief Guest pointer handle to SetMousePos function. */
			GuestHandle SetMousePos;

		} initGuest;
	} data;
};

#pragma pack(pop)
