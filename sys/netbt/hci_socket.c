/*	$NetBSD: hci_socket.c,v 1.24 2014/05/20 19:04:00 rmind Exp $	*/

/*-
 * Copyright (c) 2005 Iain Hibbert.
 * Copyright (c) 2006 Itronix Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of Itronix Inc. may not be used to endorse
 *    or promote products derived from this software without specific
 *    prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ITRONIX INC. ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ITRONIX INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: hci_socket.c,v 1.24 2014/05/20 19:04:00 rmind Exp $");

/* load symbolic names */
#ifdef BLUETOOTH_DEBUG
#define PRUREQUESTS
#define PRCOREQUESTS
#endif

#include <sys/param.h>
#include <sys/domain.h>
#include <sys/kauth.h>
#include <sys/kernel.h>
#include <sys/kmem.h>
#include <sys/mbuf.h>
#include <sys/proc.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/systm.h>

#include <netbt/bluetooth.h>
#include <netbt/hci.h>

/*******************************************************************************
 *
 * HCI SOCK_RAW Sockets - for control of Bluetooth Devices
 *
 */

/*
 * the raw HCI protocol control block
 */
struct hci_pcb {
	struct socket		*hp_socket;	/* socket */
	kauth_cred_t		hp_cred;	/* owner credential */
	unsigned int		hp_flags;	/* flags */
	bdaddr_t		hp_laddr;	/* local address */
	bdaddr_t		hp_raddr;	/* remote address */
	struct hci_filter	hp_efilter;	/* user event filter */
	struct hci_filter	hp_pfilter;	/* user packet filter */
	LIST_ENTRY(hci_pcb)	hp_next;	/* next HCI pcb */
};

/* hp_flags */
#define HCI_DIRECTION		(1<<1)	/* direction control messages */
#define HCI_PROMISCUOUS		(1<<2)	/* listen to all units */

LIST_HEAD(hci_pcb_list, hci_pcb) hci_pcb = LIST_HEAD_INITIALIZER(hci_pcb);

/* sysctl defaults */
int hci_sendspace = HCI_CMD_PKT_SIZE;
int hci_recvspace = 4096;

/* unprivileged commands opcode table */
static const struct {
	uint16_t	opcode;
	uint8_t		offs;	/* 0 - 63 */
	uint8_t		mask;	/* bit 0 - 7 */
	uint8_t		length;	/* approved length */
} hci_cmds[] = {
	{ HCI_CMD_INQUIRY,
	  0,  0x01, sizeof(hci_inquiry_cp) },
	{ HCI_CMD_REMOTE_NAME_REQ,
	  2,  0x08, sizeof(hci_remote_name_req_cp) },
	{ HCI_CMD_READ_REMOTE_FEATURES,
	  2,  0x20, sizeof(hci_read_remote_features_cp) },
	{ HCI_CMD_READ_REMOTE_EXTENDED_FEATURES,
	  2,  0x40, sizeof(hci_read_remote_extended_features_cp) },
	{ HCI_CMD_READ_REMOTE_VER_INFO,
	  2,  0x80, sizeof(hci_read_remote_ver_info_cp) },
	{ HCI_CMD_READ_CLOCK_OFFSET,
	  3,  0x01, sizeof(hci_read_clock_offset_cp) },
	{ HCI_CMD_READ_LMP_HANDLE,
	  3,  0x02, sizeof(hci_read_lmp_handle_cp) },
	{ HCI_CMD_ROLE_DISCOVERY,
	  4,  0x80, sizeof(hci_role_discovery_cp) },
	{ HCI_CMD_READ_LINK_POLICY_SETTINGS,
	  5,  0x02, sizeof(hci_read_link_policy_settings_cp) },
	{ HCI_CMD_READ_DEFAULT_LINK_POLICY_SETTINGS,
	  5,  0x08, 0 },
	{ HCI_CMD_READ_PIN_TYPE,
	  6,  0x04, 0 },
	{ HCI_CMD_READ_LOCAL_NAME,
	  7,  0x02, 0 },
	{ HCI_CMD_READ_CON_ACCEPT_TIMEOUT,
	  7,  0x04, 0 },
	{ HCI_CMD_READ_PAGE_TIMEOUT,
	  7,  0x10, 0 },
	{ HCI_CMD_READ_SCAN_ENABLE,
	  7,  0x40, 0 },
	{ HCI_CMD_READ_PAGE_SCAN_ACTIVITY,
	  8,  0x01, 0 },
	{ HCI_CMD_READ_INQUIRY_SCAN_ACTIVITY,
	  8,  0x04, 0 },
	{ HCI_CMD_READ_AUTH_ENABLE,
	  8,  0x10, 0 },
	{ HCI_CMD_READ_ENCRYPTION_MODE,
	  8,  0x40, 0 },
	{ HCI_CMD_READ_UNIT_CLASS,
	  9,  0x01, 0 },
	{ HCI_CMD_READ_VOICE_SETTING,
	  9,  0x04, 0 },
	{ HCI_CMD_READ_AUTO_FLUSH_TIMEOUT,
	  9,  0x10, sizeof(hci_read_auto_flush_timeout_cp) },
	{ HCI_CMD_READ_NUM_BROADCAST_RETRANS,
	  9,  0x40, 0 },
	{ HCI_CMD_READ_HOLD_MODE_ACTIVITY,
	  10, 0x01, 0 },
	{ HCI_CMD_READ_XMIT_LEVEL,
	  10, 0x04, sizeof(hci_read_xmit_level_cp) },
	{ HCI_CMD_READ_SCO_FLOW_CONTROL,
	  10, 0x08, 0 },
	{ HCI_CMD_READ_LINK_SUPERVISION_TIMEOUT,
	  11, 0x01, sizeof(hci_read_link_supervision_timeout_cp) },
	{ HCI_CMD_READ_NUM_SUPPORTED_IAC,
	  11, 0x04, 0 },
	{ HCI_CMD_READ_IAC_LAP,
	  11, 0x08, 0 },
	{ HCI_CMD_READ_PAGE_SCAN_PERIOD,
	  11, 0x20, 0 },
	{ HCI_CMD_READ_PAGE_SCAN,
	  11, 0x80, 0 },
	{ HCI_CMD_READ_INQUIRY_SCAN_TYPE,
	  12, 0x10, 0 },
	{ HCI_CMD_READ_INQUIRY_MODE,
	  12, 0x40, 0 },
	{ HCI_CMD_READ_PAGE_SCAN_TYPE,
	  13, 0x01, 0 },
	{ HCI_CMD_READ_AFH_ASSESSMENT,
	  13, 0x04, 0 },
	{ HCI_CMD_READ_LOCAL_VER,
	  14, 0x08, 0 },
	{ HCI_CMD_READ_LOCAL_COMMANDS,
	  14, 0x10, 0 },
	{ HCI_CMD_READ_LOCAL_FEATURES,
	  14, 0x20, 0 },
	{ HCI_CMD_READ_LOCAL_EXTENDED_FEATURES,
	  14, 0x40, sizeof(hci_read_local_extended_features_cp) },
	{ HCI_CMD_READ_BUFFER_SIZE,
	  14, 0x80, 0 },
	{ HCI_CMD_READ_COUNTRY_CODE,
	  15, 0x01, 0 },
	{ HCI_CMD_READ_BDADDR,
	  15, 0x02, 0 },
	{ HCI_CMD_READ_FAILED_CONTACT_CNTR,
	  15, 0x04, sizeof(hci_read_failed_contact_cntr_cp) },
	{ HCI_CMD_READ_LINK_QUALITY,
	  15, 0x10, sizeof(hci_read_link_quality_cp) },
	{ HCI_CMD_READ_RSSI,
	  15, 0x20, sizeof(hci_read_rssi_cp) },
	{ HCI_CMD_READ_AFH_CHANNEL_MAP,
	  15, 0x40, sizeof(hci_read_afh_channel_map_cp) },
	{ HCI_CMD_READ_CLOCK,
	  15, 0x80, sizeof(hci_read_clock_cp) },
	{ HCI_CMD_READ_LOOPBACK_MODE,
	  16, 0x01, 0 },
	{ HCI_CMD_READ_EXTENDED_INQUIRY_RSP,
	  17, 0x01, 0 },
	{ HCI_CMD_READ_SIMPLE_PAIRING_MODE,
	  17, 0x20, 0 },
	{ HCI_CMD_READ_INQUIRY_RSP_XMIT_POWER,
	  18, 0x01, 0 },
	{ HCI_CMD_READ_DEFAULT_ERRDATA_REPORTING,
	  18, 0x04, 0 },
};

/*
 * supply a basic device send/recv policy
 */
static int
hci_device_cb(kauth_cred_t cred, kauth_action_t action, void *cookie,
    void *arg0, void *arg1, void *arg2, void *arg3)
{
	int i, result;

	result = KAUTH_RESULT_DEFER;

	switch (action) {
	case KAUTH_DEVICE_BLUETOOTH_SEND: {
		struct hci_unit *unit = (struct hci_unit *)arg0;
		hci_cmd_hdr_t *hdr = (hci_cmd_hdr_t *)arg1;

		/*
		 * Allow sending unprivileged commands if the packet size
		 * is correct and the unit claims to support it
		 */

		if (hdr->type != HCI_CMD_PKT)
			break;

		for (i = 0; i < __arraycount(hci_cmds); i++) {
			if (hdr->opcode == hci_cmds[i].opcode
			    && hdr->length == hci_cmds[i].length
			    && (unit->hci_cmds[hci_cmds[i].offs] & hci_cmds[i].mask)) {
				result = KAUTH_RESULT_ALLOW;
				break;
			}
		}

		break;
		}

	case KAUTH_DEVICE_BLUETOOTH_RECV:
		switch((uint8_t)(uintptr_t)arg0) {
		case HCI_CMD_PKT: {
			uint16_t opcode = (uint16_t)(uintptr_t)arg1;

			/*
			 * Allow to see any unprivileged command packet
			 */

			for (i = 0; i < __arraycount(hci_cmds); i++) {
				if (opcode == hci_cmds[i].opcode) {
					result = KAUTH_RESULT_ALLOW;
					break;
				}
			}

			break;
			}

		case HCI_EVENT_PKT: {
			uint8_t event = (uint8_t)(uintptr_t)arg1;

			/*
			 * Allow to receive most events
			 */

			switch (event) {
			case HCI_EVENT_RETURN_LINK_KEYS:
			case HCI_EVENT_LINK_KEY_NOTIFICATION:
			case HCI_EVENT_USER_CONFIRM_REQ:
			case HCI_EVENT_USER_PASSKEY_NOTIFICATION:
			case HCI_EVENT_VENDOR:
				break;

			default:
				result = KAUTH_RESULT_ALLOW;
				break;
			}

		    	break;
			}

		case HCI_ACL_DATA_PKT:
		case HCI_SCO_DATA_PKT: {
			/* uint16_t handle = (uint16_t)(uintptr_t)arg1; */
			/*
			 * don't normally allow receiving data packets
			 */
			break;
			}

		default:
			break;
		}

		break;

	default:
		break;
	}

	return result;
}

/*
 * HCI protocol init routine,
 * - set up a kauth listener to provide basic packet access policy
 */
void
hci_init(void)
{

	if (kauth_listen_scope(KAUTH_SCOPE_DEVICE, hci_device_cb, NULL) == NULL)
		panic("Bluetooth HCI: cannot listen on device scope");
}

/*
 * When command packet reaches the device, we can drop
 * it from the socket buffer (called from hci_output_acl)
 */
void
hci_drop(void *arg)
{
	struct socket *so = arg;

	sbdroprecord(&so->so_snd);
	sowwakeup(so);
}

/*
 * HCI socket is going away and has some pending packets. We let them
 * go by design, but remove the context pointer as it will be invalid
 * and we no longer need to be notified.
 */
static void
hci_cmdwait_flush(struct socket *so)
{
	struct hci_unit *unit;
	struct socket *ctx;
	struct mbuf *m;

	DPRINTF("flushing %p\n", so);

	SIMPLEQ_FOREACH(unit, &hci_unit_list, hci_next) {
		m = MBUFQ_FIRST(&unit->hci_cmdwait);
		while (m != NULL) {
			ctx = M_GETCTX(m, struct socket *);
			if (ctx == so)
				M_SETCTX(m, NULL);

			m = MBUFQ_NEXT(m);
		}
	}
}

/*
 * HCI send packet
 *     This came from userland, so check it out.
 */
static int
hci_send(struct hci_pcb *pcb, struct mbuf *m, bdaddr_t *addr)
{
	struct hci_unit *unit;
	struct mbuf *m0;
	hci_cmd_hdr_t hdr;
	int err;

	KASSERT(m != NULL);
	KASSERT(addr != NULL);

	/* wants at least a header to start with */
	if (m->m_pkthdr.len < sizeof(hdr)) {
		err = EMSGSIZE;
		goto bad;
	}
	m_copydata(m, 0, sizeof(hdr), &hdr);
	hdr.opcode = le16toh(hdr.opcode);

	/* only allows CMD packets to be sent */
	if (hdr.type != HCI_CMD_PKT) {
		err = EINVAL;
		goto bad;
	}

	/* validates packet length */
	if (m->m_pkthdr.len != sizeof(hdr) + hdr.length) {
		err = EMSGSIZE;
		goto bad;
	}

	/* finds destination */
	unit = hci_unit_lookup(addr);
	if (unit == NULL) {
		err = ENETDOWN;
		goto bad;
	}

	/* security checks for unprivileged users */
	if (pcb->hp_cred != NULL
	    && kauth_authorize_device(pcb->hp_cred,
	    KAUTH_DEVICE_BLUETOOTH_SEND,
	    unit, &hdr, NULL, NULL) != 0) {
		err = EPERM;
		goto bad;
	}

	/* makess a copy for precious to keep */
	m0 = m_copypacket(m, M_DONTWAIT);
	if (m0 == NULL) {
		err = ENOMEM;
		goto bad;
	}
	sbappendrecord(&pcb->hp_socket->so_snd, m0);
	M_SETCTX(m, pcb->hp_socket);	/* enable drop callback */

	DPRINTFN(2, "(%s) opcode (%03x|%04x)\n", device_xname(unit->hci_dev),
		HCI_OGF(hdr.opcode), HCI_OCF(hdr.opcode));

	/* Sendss it */
	if (unit->hci_num_cmd_pkts == 0)
		MBUFQ_ENQUEUE(&unit->hci_cmdwait, m);
	else
		hci_output_cmd(unit, m);

	return 0;

bad:
	DPRINTF("packet (%d bytes) not sent (error %d)\n",
			m->m_pkthdr.len, err);
	if (m) m_freem(m);
	return err;
}

static int
hci_attach(struct socket *so, int proto)
{
	struct hci_pcb *pcb;
	int error;

	KASSERT(so->so_pcb == NULL);

	if (so->so_lock == NULL) {
		mutex_obj_hold(bt_lock);
		so->so_lock = bt_lock;
		solock(so);
	}
	KASSERT(solocked(so));

	error = soreserve(so, hci_sendspace, hci_recvspace);
	if (error) {
		return error;
	}

	pcb = kmem_zalloc(sizeof(struct hci_pcb), KM_SLEEP);
	pcb->hp_cred = kauth_cred_dup(curlwp->l_cred);
	pcb->hp_socket = so;

	/*
	 * Set default user filter. By default, socket only passes
	 * Command_Complete and Command_Status Events.
	 */
	hci_filter_set(HCI_EVENT_COMMAND_COMPL, &pcb->hp_efilter);
	hci_filter_set(HCI_EVENT_COMMAND_STATUS, &pcb->hp_efilter);
	hci_filter_set(HCI_EVENT_PKT, &pcb->hp_pfilter);

	LIST_INSERT_HEAD(&hci_pcb, pcb, hp_next);
	so->so_pcb = pcb;

	return 0;
}

static void
hci_detach(struct socket *so)
{
	struct hci_pcb *pcb;

	pcb = (struct hci_pcb *)so->so_pcb;
	KASSERT(pcb != NULL);

	if (so->so_snd.sb_mb != NULL)
		hci_cmdwait_flush(so);

	if (pcb->hp_cred != NULL)
		kauth_cred_free(pcb->hp_cred);

	so->so_pcb = NULL;
	LIST_REMOVE(pcb, hp_next);
	kmem_free(pcb, sizeof(*pcb));
}

/*
 * User Request.
 * up is socket
 * m is either
 *	optional mbuf chain containing message
 *	ioctl command (PRU_CONTROL)
 * nam is either
 *	optional mbuf chain containing an address
 *	ioctl data (PRU_CONTROL)
 * ctl is optional mbuf chain containing socket options
 * l is pointer to process requesting action (if any)
 *
 * we are responsible for disposing of m and ctl if
 * they are mbuf chains
 */
static int
hci_usrreq(struct socket *up, int req, struct mbuf *m,
		struct mbuf *nam, struct mbuf *ctl, struct lwp *l)
{
	struct hci_pcb *pcb = (struct hci_pcb *)up->so_pcb;
	struct sockaddr_bt *sa;
	int err = 0;

	DPRINTFN(2, "%s\n", prurequests[req]);
	KASSERT(req != PRU_ATTACH);
	KASSERT(req != PRU_DETACH);

	switch(req) {
	case PRU_CONTROL:
		mutex_enter(bt_lock);
		err = hci_ioctl((unsigned long)m, (void *)nam, l);
		mutex_exit(bt_lock);
		return err;

	case PRU_PURGEIF:
		return EOPNOTSUPP;
	}

	/* anything after here *requires* a pcb */
	if (pcb == NULL) {
		err = EINVAL;
		goto release;
	}

	switch(req) {
	case PRU_DISCONNECT:
		bdaddr_copy(&pcb->hp_raddr, BDADDR_ANY);

		/* XXX we cannot call soisdisconnected() here, as it sets
		 * SS_CANTRCVMORE and SS_CANTSENDMORE. The problem being,
		 * that soisconnected() does not clear these and if you
		 * try to reconnect this socket (which is permitted) you
		 * get a broken pipe when you try to write any data.
		 */
		up->so_state &= ~SS_ISCONNECTED;
		break;

	case PRU_ABORT:
		soisdisconnected(up);
		hci_detach(up);
		return 0;

	case PRU_BIND:
		KASSERT(nam != NULL);
		sa = mtod(nam, struct sockaddr_bt *);

		if (sa->bt_len != sizeof(struct sockaddr_bt))
			return EINVAL;

		if (sa->bt_family != AF_BLUETOOTH)
			return EAFNOSUPPORT;

		bdaddr_copy(&pcb->hp_laddr, &sa->bt_bdaddr);

		if (bdaddr_any(&sa->bt_bdaddr))
			pcb->hp_flags |= HCI_PROMISCUOUS;
		else
			pcb->hp_flags &= ~HCI_PROMISCUOUS;

		return 0;

	case PRU_CONNECT:
		KASSERT(nam != NULL);
		sa = mtod(nam, struct sockaddr_bt *);

		if (sa->bt_len != sizeof(struct sockaddr_bt))
			return EINVAL;

		if (sa->bt_family != AF_BLUETOOTH)
			return EAFNOSUPPORT;

		if (hci_unit_lookup(&sa->bt_bdaddr) == NULL)
			return EADDRNOTAVAIL;

		bdaddr_copy(&pcb->hp_raddr, &sa->bt_bdaddr);
		soisconnected(up);
		return 0;

	case PRU_PEERADDR:
		KASSERT(nam != NULL);
		sa = mtod(nam, struct sockaddr_bt *);

		memset(sa, 0, sizeof(struct sockaddr_bt));
		nam->m_len =
		sa->bt_len = sizeof(struct sockaddr_bt);
		sa->bt_family = AF_BLUETOOTH;
		bdaddr_copy(&sa->bt_bdaddr, &pcb->hp_raddr);
		return 0;

	case PRU_SOCKADDR:
		KASSERT(nam != NULL);
		sa = mtod(nam, struct sockaddr_bt *);

		memset(sa, 0, sizeof(struct sockaddr_bt));
		nam->m_len =
		sa->bt_len = sizeof(struct sockaddr_bt);
		sa->bt_family = AF_BLUETOOTH;
		bdaddr_copy(&sa->bt_bdaddr, &pcb->hp_laddr);
		return 0;

	case PRU_SHUTDOWN:
		socantsendmore(up);
		break;

	case PRU_SEND:
		sa = NULL;
		if (nam) {
			sa = mtod(nam, struct sockaddr_bt *);

			if (sa->bt_len != sizeof(struct sockaddr_bt)) {
				err = EINVAL;
				goto release;
			}

			if (sa->bt_family != AF_BLUETOOTH) {
				err = EAFNOSUPPORT;
				goto release;
			}
		}

		if (ctl) /* have no use for this */
			m_freem(ctl);

		return hci_send(pcb, m, (sa ? &sa->bt_bdaddr : &pcb->hp_raddr));

	case PRU_SENSE:
		return 0;		/* (no sense - Doh!) */

	case PRU_RCVD:
	case PRU_RCVOOB:
		return EOPNOTSUPP;	/* (no release) */

	case PRU_ACCEPT:
	case PRU_CONNECT2:
	case PRU_LISTEN:
	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:
		err = EOPNOTSUPP;
		break;

	default:
		UNKNOWN(req);
		err = EOPNOTSUPP;
		break;
	}

release:
	if (m)
		m_freem(m);
	if (ctl)
		m_freem(ctl);
	return err;
}

/*
 * get/set socket options
 */
int
hci_ctloutput(int req, struct socket *so, struct sockopt *sopt)
{
	struct hci_pcb *pcb = (struct hci_pcb *)so->so_pcb;
	int optval, err = 0;

	DPRINTFN(2, "req %s\n", prcorequests[req]);

	if (pcb == NULL)
		return EINVAL;

	if (sopt->sopt_level != BTPROTO_HCI)
		return ENOPROTOOPT;

	switch(req) {
	case PRCO_GETOPT:
		switch (sopt->sopt_name) {
		case SO_HCI_EVT_FILTER:
			err = sockopt_set(sopt, &pcb->hp_efilter,
			    sizeof(struct hci_filter));

			break;

		case SO_HCI_PKT_FILTER:
			err = sockopt_set(sopt, &pcb->hp_pfilter,
			    sizeof(struct hci_filter));

			break;

		case SO_HCI_DIRECTION:
			err = sockopt_setint(sopt,
			    (pcb->hp_flags & HCI_DIRECTION ? 1 : 0));

			break;

		default:
			err = ENOPROTOOPT;
			break;
		}
		break;

	case PRCO_SETOPT:
		switch (sopt->sopt_name) {
		case SO_HCI_EVT_FILTER:	/* set event filter */
			err = sockopt_get(sopt, &pcb->hp_efilter,
			    sizeof(pcb->hp_efilter));

			break;

		case SO_HCI_PKT_FILTER:	/* set packet filter */
			err = sockopt_get(sopt, &pcb->hp_pfilter,
			    sizeof(pcb->hp_pfilter));

			break;

		case SO_HCI_DIRECTION:	/* request direction ctl messages */
			err = sockopt_getint(sopt, &optval);
			if (err)
				break;

			if (optval)
				pcb->hp_flags |= HCI_DIRECTION;
			else
				pcb->hp_flags &= ~HCI_DIRECTION;
			break;

		default:
			err = ENOPROTOOPT;
			break;
		}
		break;

	default:
		err = ENOPROTOOPT;
		break;
	}

	return err;
}

/*
 * HCI mbuf tap routine
 *
 * copy packets to any raw HCI sockets that wish (and are
 * permitted) to see them
 */
void
hci_mtap(struct mbuf *m, struct hci_unit *unit)
{
	struct hci_pcb *pcb;
	struct mbuf *m0, *ctlmsg, **ctl;
	struct sockaddr_bt sa;
	uint8_t type;
	uint8_t event;
	uint16_t arg1;

	KASSERT(m->m_len >= sizeof(type));

	type = *mtod(m, uint8_t *);

	memset(&sa, 0, sizeof(sa));
	sa.bt_len = sizeof(struct sockaddr_bt);
	sa.bt_family = AF_BLUETOOTH;
	bdaddr_copy(&sa.bt_bdaddr, &unit->hci_bdaddr);

	LIST_FOREACH(pcb, &hci_pcb, hp_next) {
		/*
		 * filter according to source address
		 */
		if ((pcb->hp_flags & HCI_PROMISCUOUS) == 0
		    && bdaddr_same(&pcb->hp_laddr, &sa.bt_bdaddr) == 0)
			continue;

		/*
		 * filter according to packet type filter
		 */
		if (hci_filter_test(type, &pcb->hp_pfilter) == 0)
			continue;

		/*
		 * filter according to event/security filters
		 */
		switch(type) {
		case HCI_EVENT_PKT:
			KASSERT(m->m_len >= sizeof(hci_event_hdr_t));

			event = mtod(m, hci_event_hdr_t *)->event;

			if (hci_filter_test(event, &pcb->hp_efilter) == 0)
				continue;

			arg1 = event;
			break;

		case HCI_CMD_PKT:
			KASSERT(m->m_len >= sizeof(hci_cmd_hdr_t));
			arg1 = le16toh(mtod(m, hci_cmd_hdr_t *)->opcode);
			break;

		case HCI_ACL_DATA_PKT:
			KASSERT(m->m_len >= sizeof(hci_acldata_hdr_t));
			arg1 = le16toh(mtod(m, hci_acldata_hdr_t *)->con_handle);
			arg1 = HCI_CON_HANDLE(arg1);
			break;

		case HCI_SCO_DATA_PKT:
			KASSERT(m->m_len >= sizeof(hci_scodata_hdr_t));
			arg1 = le16toh(mtod(m, hci_scodata_hdr_t *)->con_handle);
			arg1 = HCI_CON_HANDLE(arg1);
			break;

		default:
			arg1 = 0;
			break;
		}

		if (pcb->hp_cred != NULL
		    && kauth_authorize_device(pcb->hp_cred,
		    KAUTH_DEVICE_BLUETOOTH_RECV,
		    KAUTH_ARG(type), KAUTH_ARG(arg1), NULL, NULL) != 0)
			continue;

		/*
		 * create control messages
		 */
		ctlmsg = NULL;
		ctl = &ctlmsg;
		if (pcb->hp_flags & HCI_DIRECTION) {
			int dir = m->m_flags & M_LINK0 ? 1 : 0;

			*ctl = sbcreatecontrol(&dir, sizeof(dir),
			    SCM_HCI_DIRECTION, BTPROTO_HCI);

			if (*ctl != NULL)
				ctl = &((*ctl)->m_next);
		}
		if (pcb->hp_socket->so_options & SO_TIMESTAMP) {
			struct timeval tv;

			microtime(&tv);
			*ctl = sbcreatecontrol(&tv, sizeof(tv),
			    SCM_TIMESTAMP, SOL_SOCKET);

			if (*ctl != NULL)
				ctl = &((*ctl)->m_next);
		}

		/*
		 * copy to socket
		 */
		m0 = m_copypacket(m, M_DONTWAIT);
		if (m0 && sbappendaddr(&pcb->hp_socket->so_rcv,
				(struct sockaddr *)&sa, m0, ctlmsg)) {
			sorwakeup(pcb->hp_socket);
		} else {
			m_freem(ctlmsg);
			m_freem(m0);
		}
	}
}

PR_WRAP_USRREQS(hci)

#define	hci_attach		hci_attach_wrapper
#define	hci_detach		hci_detach_wrapper
#define	hci_usrreq		hci_usrreq_wrapper

const struct pr_usrreqs hci_usrreqs = {
	.pr_attach	= hci_attach,
	.pr_detach	= hci_detach,
	.pr_generic	= hci_usrreq,
};
