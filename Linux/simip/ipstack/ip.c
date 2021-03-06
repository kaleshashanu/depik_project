
#include "config.h"
#include "osdef.h"
#include "osif.h"
#include "ethdrv.h"
#include "nwstk.h"

#define LOCAL static
LOCAL UINT32 ipPktCnt, ipPktTcp, ipPktUdp, ipPktIcmp, ipPktUnKnown;

void ipHhtons(ipHdr_t *ipHdr);
void ipHntohs(ipHdr_t *ipHdr);
/*******************************************************************************
* Name:
* Description:
*******************************************************************************/
int ipIn(pktBuf_t *pkt)
{
  int len;
  ipHdr_t *ipHdr = (ipHdr_t *) &pkt->buf[14];

  ipPktCnt++;

  /*** Validate version and checksum of IP header ***/
  /*** If dst IP addrs of pkt does not match, drop the packet **/
  /*** If IP packet is a fragment, drop it **/

  /*** If length specified IP header is different from length given by pktBuf,
   *** then set length in pktBuf to length given in IP header ***/

    len = htons(ipHdr->pktLen);
  //ipHntohs(ipHdr);
  //pkt->len = ipHdr->pktLen + 14;
  pkt->len = pkt->len - 14;

  switch(ipHdr->proto)
  {
    case PROTO_TCP://6
      ipPktTcp++;
      //tcpIn(pkt);
      pktFree(pkt);
      break;

    case PROTO_UDP://17
      ipPktUdp++;
      udpIn(pkt);
      break;

    case PROTO_ICMP://1
      ipPktIcmp++;
      icmpIn(pkt);
      break;

    default:
      ipPktUnKnown++;
      pktFree(pkt);
  }
}

/*******************************************************************************
* Name:
* Description:
*******************************************************************************/
UINT16 computeChecksum(UINT8 *ptr,int len)
{
  UINT16 *pShort,check= 0;
  UINT16 temp = 0;
  UINT16 word = 0,tempword = 0;
  unsigned int result=0;
  int i;

  pShort = (UINT16 *) ptr;
  for(i=0;i<len/2;i++)
  {
     result+= *pShort++;
  }
   
  result= (result>>16) + ( result & 0xffff );
  result += (result>>16);
  result = (result & 0xffff) ;
  temp = result;
   //printf("Compute Checksum  %x\n",~temp);
   return ~result;
}
    
/*******************************************************************************
* Name:
* Description:
*******************************************************************************/
void ipSendPkt(pktBuf_t *pkt, UINT8 *dstAddr, UINT8 protocol)
{
  static UINT16  ipPktId;

  iface_t   *pIf;
  ipHdr_t   *ipHdr   = (ipHdr_t  *)&pkt->buf[ETH_HDR_LEN];
  ethHdr_t  *ethHdr  = (ethHdr_t *)&pkt->buf[0];

  ipHdr->verlen   = 0x45;
  ipHdr->tos      = 0x00;
  //ipHdr->pktLen   = htons(pkt->len + IP_HDR_LEN);
  ipHdr->pktLen   = pkt->len + IP_HDR_LEN;
  ipHdr->pktId    = ipPktId++;
  ipHdr->flagFrag = 0x0000;
  ipHdr->cksum    = 0x0000;
  ipHdr->ttl      = 0xff;
  ipHdr->proto    = protocol;
  memcpy(ipHdr->dstAdrs,dstAddr,4);
    
  /** rtindex = routeGet(dstAddr);   **/
  pIf = ifaceTbl[1];
  memcpy(ipHdr->srcAdrs,pIf->ipAdrs,4);
  memcpy(ethHdr->srcHwa,pIf->macAdrs,6);
  ethHdr->type = htons(IP_PKT);
  ipHhtons(ipHdr);
  ipHdr->cksum = computeChecksum((UINT8 *) ipHdr, IP_HDR_LEN );
  pkt->len = pkt->len + IP_HDR_LEN + ETH_HDR_LEN;
  if((strcmp(ipHdr->dstAdrs,pIf->ipAdrs)==0)||
                ( strcmp(ipHdr->dstAdrs,"127.0.0.1")==0))
   {
      printf("this is loop back ip address\n" );
      printf("\n the ip address %s",ipHdr->dstAdrs);
      memcpy(ethHdr->dstHwa,pIf->macAdrs,6);
      pIf->sendFP(pIf->pDev,pkt); 
//     ipIn(pkt);    
     }
  else 
      arpSendIpPkt(pkt, dstAddr, pIf);
}

/*******************************************************************************
* Name:
* Description:
*******************************************************************************/
LOCAL int ipStat(int argc, char * argv[])
{
  printf("IP Pkts  : %d\n"
         "TCP Pkts : %d\n"
         "UDP Pkts : %d\n"
         "ICMP PKts: %d\n"
         "Unknown  : %d\n",
         ipPktCnt, ipPktTcp, ipPktUdp, ipPktIcmp, ipPktUnKnown);
}

/*******************************************************************************
* Name:
* Description:
*******************************************************************************/
ipInit()
{
  //shellRegCmd("ipstat",ipStat, "IP Counters");
}

/******************************************************************************
 * Name :
 * Description :
 ******************************************************************************/
void ipHhtons(ipHdr_t *ipHdr)
{
  ipHdr->pktLen = htons(ipHdr->pktLen);
}
/******************************************************************************
 * Name :
 * Description :
 ******************************************************************************/
void ipHntohs(ipHdr_t *ipHdr)
{
  ipHdr->pktLen = htons(ipHdr->pktLen);
}
