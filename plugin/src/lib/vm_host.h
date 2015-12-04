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
#define VM_UNKNOWN_ERROR                -999 /**< Unknow error. */
#define VM_CUSTOM_VM_ERROR			   -1000 /**< First specific error code of virtual machine. */


#ifdef __cplusplus
	#define typedef
	namespace vm { namespace type {
#endif

/**
 * \struct VirtualMachineInfo
 * \brief Virtual machine information structure.
 */
typedef struct VirtualMachineInfo
{
	int    structSize;		 /**< = sizeof(VirtualMachineInfo). */
	char * name;			 /**< Virtual machine name. */
	int    vm_version_major; /**< Virtual machine major version number. */
	int    vm_version_minor; /**< Virtual machine minor version number. */
} ; 

typedef struct VirtualMachine
{
	int  structSize; /**< = sizeof(VirtualMachine). */

    /**
	 * \brief Get virtual machine informations.
	 * \return Virtual machine informations
     */
	const VirtualMachineInfo (*getVmInfo)      ();

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
     * \param Name of parameter.
     * \return Parameter's value or NULL;
     */
	const char * (*getParameter) (const char * parameter);
};

#ifdef __cplusplus
	}	}
	#undef typedef
#endif
