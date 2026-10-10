/*
WIIU_NET_NULL.C

The Xbox network (Winsock and XNet) and the port's internet play
(port/linux/src/xnet.c and p2p*.c are the Linux build's), for the Wii U
build until it has them: the network starts, has no cable plugged in, and
no socket can be made, so the game offers no network play.
*/

#include "platform.h"
#include "p2p.h"

#include <string.h>

static int last_error;

static int network_down(void)
{
	last_error = WSAENETDOWN;
	return SOCKET_ERROR;
}

int WSAAPI WSAStartup(WORD version_requested, LPWSADATA data)
{
	if (data)
	{
		memset(data, 0, sizeof(*data));
		data->wVersion = version_requested;
		data->wHighVersion = MAKEWORD(2, 2);
	}
	return 0;
}

int WSAAPI WSACleanup(void)
{
	return 0;
}

int WSAAPI WSAGetLastError(void)
{
	return last_error;
}

INT WSAAPI XNetStartup(const XNetStartupParams *parameters)
{
	(void)parameters;
	return 0;
}

INT WSAAPI XNetCleanup(void)
{
	return 0;
}

DWORD WSAAPI XNetGetEthernetLinkStatus(void)
{
	return 0;
}

DWORD WSAAPI XNetGetTitleXnAddr(XNADDR *address)
{
	memset(address, 0, sizeof(*address));
	return XNET_GET_XNADDR_NONE;
}

/* (not secret: nothing is sent with it) */
INT WSAAPI XNetRandom(BYTE *buffer, UINT size)
{
	static unsigned long state = 0x2545F491UL;
	UINT index;

	for (index = 0; index < size; index++)
	{
		state ^= state << 13;
		state ^= state >> 17;
		state ^= state << 5;
		buffer[index] = (BYTE)state;
	}
	return 0;
}

INT WSAAPI XNetCreateKey(XNKID *key_identifier, XNKEY *key)
{
	XNetRandom((BYTE *)key_identifier, sizeof(*key_identifier));
	XNetRandom((BYTE *)key, sizeof(*key));
	return 0;
}

INT WSAAPI XNetRegisterKey(const XNKID *key_identifier, const XNKEY *key)
{
	(void)key_identifier;
	(void)key;
	return 0;
}

INT WSAAPI XNetUnregisterKey(const XNKID *key_identifier)
{
	(void)key_identifier;
	return 0;
}

INT WSAAPI XNetXnAddrToInAddr(const XNADDR *address, const XNKID *key_identifier, IN_ADDR *result)
{
	(void)key_identifier;
	*result = address->ina;
	return 0;
}

/* ---------- sockets: none */

SOCKET WSAAPI halo_ws_socket(int family, int type, int protocol)
{
	(void)family;
	(void)type;
	(void)protocol;
	last_error = WSAENETDOWN;
	return INVALID_SOCKET;
}

SOCKET WSAAPI halo_ws_accept(SOCKET socket, struct sockaddr *address, int *address_length)
{
	(void)socket;
	(void)address;
	(void)address_length;
	last_error = WSAENETDOWN;
	return INVALID_SOCKET;
}

int WSAAPI halo_ws_closesocket(SOCKET socket) { (void)socket; return 0; }

int WSAAPI halo_ws_bind(SOCKET socket, const struct sockaddr *address, int address_length)
{
	(void)socket; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_connect(SOCKET socket, const struct sockaddr *address, int address_length)
{
	(void)socket; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_listen(SOCKET socket, int backlog)
{
	(void)socket; (void)backlog;
	return network_down();
}

int WSAAPI halo_ws_getpeername(SOCKET socket, struct sockaddr *address, int *address_length)
{
	(void)socket; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_getsockname(SOCKET socket, struct sockaddr *address, int *address_length)
{
	(void)socket; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_getsockopt(SOCKET socket, int level, int name, char *value, int *length)
{
	(void)socket; (void)level; (void)name; (void)value; (void)length;
	return network_down();
}

int WSAAPI halo_ws_setsockopt(SOCKET socket, int level, int name, const char *value, int length)
{
	(void)socket; (void)level; (void)name; (void)value; (void)length;
	return network_down();
}

int WSAAPI halo_ws_ioctlsocket(SOCKET socket, long command, u_long *argument)
{
	(void)socket; (void)command; (void)argument;
	return network_down();
}

int WSAAPI halo_ws_recv(SOCKET socket, char *buffer, int length, int flags)
{
	(void)socket; (void)buffer; (void)length; (void)flags;
	return network_down();
}

int WSAAPI halo_ws_recvfrom(SOCKET socket, char *buffer, int length, int flags, struct sockaddr *address,
	int *address_length)
{
	(void)socket; (void)buffer; (void)length; (void)flags; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_send(SOCKET socket, const char *buffer, int length, int flags)
{
	(void)socket; (void)buffer; (void)length; (void)flags;
	return network_down();
}

int WSAAPI halo_ws_sendto(SOCKET socket, const char *buffer, int length, int flags, const struct sockaddr *address,
	int address_length)
{
	(void)socket; (void)buffer; (void)length; (void)flags; (void)address; (void)address_length;
	return network_down();
}

int WSAAPI halo_ws_select(int descriptor_count, halo_ws_fd_set *read_set, halo_ws_fd_set *write_set,
	halo_ws_fd_set *error_set, const struct halo_ws_timeval *timeout)
{
	(void)descriptor_count;
	(void)timeout;
	if (read_set)
		read_set->fd_count = 0;
	if (write_set)
		write_set->fd_count = 0;
	if (error_set)
		error_set->fd_count = 0;
	return 0;
}

int PASCAL __WSAFDIsSet(SOCKET socket, halo_ws_fd_set *set)
{
	(void)socket;
	(void)set;
	return 0;
}

/* ---------- the port's internet play: off */

int p2p_join_invite(const char *text) { (void)text; return 0; }
int p2p_invite_link(char *link, int size)
{
	if (size > 0)
		link[0] = 0;
	return 0;
}
int p2p_peer_address(const unsigned char *identifier, unsigned long *address)
{
	(void)identifier;
	(void)address;
	return 0;
}
unsigned long p2p_peer_endpoint_address(unsigned long virtual_address) { (void)virtual_address; return 0; }
void p2p_set_hosting_allowed(int allowed) { (void)allowed; }
void p2p_set_game_player_counts(int count, int maximum) { (void)count; (void)maximum; }
void p2p_set_hosting_public(int public) { (void)public; }
void p2p_set_hosting_password(const char *password) { (void)password; }
void p2p_set_game_listing(const char *name, const char *map, const char *gametype, int engine_type, int open,
	int in_progress, int has_teams)
{
	(void)name; (void)map; (void)gametype; (void)engine_type; (void)open; (void)in_progress; (void)has_teams;
}
void p2p_lobby_browse(int on) { (void)on; }
void p2p_lobby_refresh(void) { }
int p2p_lobby_games(struct p2p_listing *games, int maximum_count) { (void)games; (void)maximum_count; return 0; }
void p2p_lobby_mark_failed(const unsigned char *identifier) { (void)identifier; }
int p2p_listing_unlock(struct p2p_listing *listing, const char *password)
{
	(void)listing;
	(void)password;
	return 0;
}

/* the text kept of a Discord user's id or name (p2p_discord.c's) */
void p2p_discord_sanitize(char *destination, int size, const char *source, int name)
{
	int length = 0;

	for (; source && *source && length < size - 1; source++)
	{
		char character = *source;

		if ((character >= '0' && character <= '9') ||
			(name && ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
				character == '_' || character == '.' || character == '-')))
		{
			destination[length++] = character;
		}
	}
	if (size > 0)
		destination[length] = 0;
}

void p2p_discord_identity(char *id, int id_size, char *name, int name_size)
{
	if (id_size > 0)
		id[0] = 0;
	if (name_size > 0)
		name[0] = 0;
}

void p2p_hardware_id(char *hex, int size)
{
	if (size > 0)
		hex[0] = 0;
}

/* (p2p.c's: lowercase hex digits only, at most 2 * P2P_HARDWARE_ID_BYTES) */
void p2p_hardware_id_sanitize(char *destination, int size, const char *source)
{
	int length = 0;

	for (; source && *source && length < size - 1 && length < 2 * P2P_HARDWARE_ID_BYTES; source++)
	{
		char character = *source >= 'A' && *source <= 'F' ? *source - 'A' + 'a' : *source;

		if ((character >= '0' && character <= '9') || (character >= 'a' && character <= 'f'))
			destination[length++] = character;
	}
	if (size > 0)
		destination[length] = 0;
}
