/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* wp_portconfig.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* system contrl for vlan port config
*
*
*******************************************************************************/
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "ws_usrinfo.h"
#include "ws_init_dbus.h"
#include "ws_nm_status.h"
#include "ws_ec.h"
#include "ws_public.h"
#include "ws_trunk.h"
#include "ws_dcli_vlan.h"

int showPortConfig();
int cgiMain()
{
    showPortConfig();
    return 0;
}

int showPortConfig()
{
    struct list *lpublic;   /*解析public.txt文件的链表头*/
    struct list *lcontrol;  /*解析help.txt文件的链表头*/
    lpublic=get_chain_head("../htdocs/text/public.txt");
    lcontrol=get_chain_head("../htdocs/text/control.txt"); 
    struct trunk_profile head,trunk,*q;
    struct port_profile *pq;
    unsigned int tmpVal[2]={0};
    int Tnum;
  FILE *fp;
  char lan[3];
  char *encry=(char *)malloc(BUF_LEN);  
  char * product=(char *)malloc(20);
  memset(product,0,20);
  char *IDForSearch=(char *)malloc(10);
  char *str;
  int i,j,k;
  k=0;
  int abletagflag[MAX_SOLT_NUM][MAX_PORT_NUM];   /*读一般的信息，对应24个端口*/
  int DefaultUntag[MAX_SOLT_NUM][MAX_PORT_NUM];  /*构成untag的可选框机理不一样，新设置了个变量用来读dbus里默认vlan的信息*/
  char port_encry[BUF_LEN];
  unsigned short IDTemp;
  struct vlan_info  v_infoByVID;
  struct vlan_info  VinfoTemp;
  int ableNum=0;
  int flavorChoicesRight[MAX_TOTAL_NUM];
  int flavorChoicesLeft[MAX_TOTAL_NUM];
  int result,result1=0,result2=0;   
  int invalid; 
  int slotTemp,portTemp,untagNum,tagNum,DefuntagNum,DeftagNum,UNslotTemp,UNportTemp;
  char VID[10]="";
    unsigned short IDSub=0;
    
  trunk.trunkId=0;
  trunk.mSlotNo=0;
  trunk.mPortNo=0;
  head.trunkId=0;
  head.mSlotNo=0;
  head.mPortNo=0;
    char *STemp=(char *)malloc(4);
    memset(STemp,0,4);
    char *PTemp=(char *)malloc(4);
    memset(PTemp,0,4);
    v_infoByVID.vlanName=(char *)malloc(21);
    VinfoTemp.vlanName=(char *)malloc(21);
    memset(VinfoTemp.vlanName,0,21);
    char TagTemp[10]="";
    char TagTempLater[10]="";
    char TagTempBuf[10]="";

    char *UNSTemp=(char *)malloc(4);
    memset(UNSTemp,0,4);
    char *UNPTemp=(char *)malloc(4);
    memset(UNSTemp,0,4);

    char *slotNo=(char *)malloc(4);
    memset(slotNo,0,4);
    char *portNo=(char *)malloc(4);
    memset(portNo,0,4);
    char *flavors[MAX_TOTAL_NUM];
    char numTempLater[4]="";
    char numTemp[4]="";
    int ableNumLater=0;
    char addlist[200];
    char deletelist[200];
    memset(addlist,0,200);
    memset(deletelist,0,200);
    char * addlistRev[MAX_TOTAL_NUM];
    char * deletelistRev[MAX_TOTAL_NUM];

    struct slot sl;        //声明存储slot 信息的结构体变量
    sl.module_status=0;     
    sl.modname=(char *)malloc(20);     //为结构体成员申请空间，假设该字段的最大长度为20
    sl.sn=(char *)malloc(20);          //为结构体成员申请空间，假设该字段的最大长度为20
    sl.hw_ver=0;
    sl.ext_slot_num=0;
    int flagSlot[4];
    
    for(i=0;i<4;i++)
    {
        for(j=0;j<24;j++)
    	{
            abletagflag[i][j]=0;   /*abletagflag 这个2维数组0表示24个端口，1表示已经配置过tag的端口  1表示已经配置过untag的端   */
            DefaultUntag[i][j]=0;
    	}
    }
    for(i=0;i<4;i++)
    	{
            flagSlot[i]=0;
    	}
    for(i=0;i<MAX_TOTAL_NUM;i++)
    	{
        v_infoByVID.slot_port_tag[i]=(char *)malloc(6);
        v_infoByVID.slot_port_untag[i]=(char *)malloc(6);
        memset(v_infoByVID.slot_port_tag[i],0,6);
        memset(v_infoByVID.slot_port_untag[i],0,6);
        addlistRev[i]=(char *)malloc(6);
        memset(addlistRev[i],0,6);
        deletelistRev[i]=(char *)malloc(6);
        memset(deletelistRev[i],0,6);
    	
        flavors[i]=(char *)malloc(6);
        memset(flavors[i],0,6);
    	
        VinfoTemp.slot_port_tag[i]=(char *)malloc(6);
        VinfoTemp.slot_port_untag[i]=(char *)malloc(6);
        memset(VinfoTemp.slot_port_tag[i],0,6);
        memset(VinfoTemp.slot_port_untag[i],0,6);
        flavorChoicesRight[i]=0;
        flavorChoicesLeft[i]=0;
    	}
    ccgi_dbus_init(); 
    product=readproductID();

	PRODUCT_PAGE_INFO Product_select_info;
	memset(&Product_select_info, 0 ,sizeof(PRODUCT_PAGE_INFO));

	Product_Adapter_for_page(&Product_select_info,product);
	//fprintf(stderr,"Product_select_info.port_total_num=%d",Product_select_info.port_total_num);
	//fprintf(stderr,"Product_select_info.port_num=%u",Product_select_info.port_num);
	//fprintf(stderr,"Product_select_info.slot_num=%u",Product_select_info.slot_num);
    /**/

    if(0==strcmp(product,"Switch7000"))
    {
	    for(i=1;i<5;i++)
    	{
            //fprintf(stderr,"test\%d",i);
            if(nm_show_hw_config(i,&sl)==CCGI_SUCCESS)     //如果读取信息成功,第i槽插入
                {
                if(sl.module_status==2)   //slot_status=RUNNING
             		{
                        flagSlot[i-1]=1;
             		}
            	}
    	}
    }
	else
	{
        for(i=1;i<5;i++)
    	{
            flagSlot[i-1]=1;
    	}
	}

      if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
      {
        memset(encry,0,BUF_LEN);
        memset(IDForSearch,0,10);
        memset(TagTemp,0,10);
        cgiFormStringNoNewlines("UN", encry, BUF_LEN); 
        cgiFormStringNoNewlines("VID", IDForSearch, 10); 
        cgiFormStringNoNewlines("Tag", TagTemp, 10);  
    
        str=dcryption(encry);
        if(str==NULL)
    	{
          ShowErrorPage(search(lpublic,"ill_user"));           /*用户非法*/
          return 0;
    	}
        memset(port_encry,0,BUF_LEN);               	/*清空临时变量*/
        memset(VID,0,10);
        memset(TagTempLater,0,10);
        memset(numTempLater,0,10);
      }

  cgiFormStringNoNewlines("encry_port",port_encry,BUF_LEN);
  cgiFormStringNoNewlines("VId",VID,10); 
    cgiFormStringNoNewlines("encry_Num",numTempLater,10);
  cgiFormStringNoNewlines("Tag_Temp",TagTempLater,10);
    cgiFormStringNoNewlines("color_addlist", addlist, 200); 
    cgiFormStringNoNewlines("color_dellist", deletelist, 200);
  cgiHeaderContentType("text/html");
  fprintf(cgiOut,"<html xmlns=\"http://www.w3.org/1999/xhtml\"><head>");
  fprintf(cgiOut,"<meta http-equiv=Content-Type content=text/html; charset=gb2312>");
  fprintf(cgiOut,"<title>%s</title>",search(lcontrol,"vlan_manage"));
  fprintf(cgiOut,"<link rel=stylesheet href=/style.css type=text/css>"\
  "</head>"\
  "<body>");


  if((fp=fopen("../htdocs/text/public.txt","r"))==NULL)      /*以只读方式打开资源文件*/
  {
      ShowAlert(search(lpublic,"error_open"));
  }
  else
  {
	  fseek(fp,4,0);					  /*将文件指针移到离文件首4个字节处，即lan=之后*/
	  fgets(lan,3,fp);		 
	  fclose(fp);
  }
    if((cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess) || (cgiFormSubmitClicked("submit_igmp_disable") == cgiFormSuccess) || (cgiFormSubmitClicked("add_route") == cgiFormSuccess) || (cgiFormSubmitClicked("del_route") == cgiFormSuccess))
    {
        strcpy(TagTempBuf,TagTempLater);
    }
    if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
      {
          strcpy(TagTempBuf,TagTemp);
      }
    else if(cgiFormSubmitClicked("add_port") == cgiFormSuccess)      /*右移添加端口操作*/
      { 
          strcpy(TagTempBuf,TagTempLater);		  
          IDSub=atoi(VID);
          if(CMD_SUCCESS==show_vlan_ByID(&v_infoByVID,IDSub,&untagNum,&tagNum))
        	  {
          /*这里读DBUS读出已经分配的TAG和UNTAG端口信息*/
              if(0==strcmp(TagTempBuf,"Untag"))
        	  {
                      int T=show_vlan_ByID(&VinfoTemp,1,&DefuntagNum,&DeftagNum);
                  if(CMD_SUCCESS==T)
        			  {
                          for(i=0;i<Product_select_info.port_total_num;i++)
            			  { 
                              if(strcmp(VinfoTemp.slot_port_untag[i],"")!=0)     /*Untag的可选端口集是DEFAULT Vlan 的分配过的Untag*/
            				  {
                                    //fprintf(stderr,"!!untag[%d]=%s",i,VinfoTemp.slot_port_untag[i]);
                                  UNSTemp=strtok(VinfoTemp.slot_port_untag[i],"/");
                                  UNslotTemp=atoi(UNSTemp);
                                  UNPTemp=strtok(NULL,"/");
                                  UNportTemp=atoi(UNPTemp);
                                  DefaultUntag[UNslotTemp-1][UNportTemp-1]=2;
								  //fprintf(stderr,"DUTAGslotTemp=%d--portTemp=%d\n",slotTemp,portTemp);
            				  }
            			  }
        			  }

        	  }
          /*读默认vlan后的信息是做可选框的，读了后以选框内信息还得读，不能用else if(...==tag)*/
              for(i=0;i<Product_select_info.port_total_num;i++)
        		  {
                      if(strcmp(v_infoByVID.slot_port_tag[i],"")!=0)
        			  {
        			
                          STemp=strtok(v_infoByVID.slot_port_tag[i],"/");
                          slotTemp=atoi(STemp);
                          PTemp=strtok(NULL,"/"); 
                          portTemp=atoi(PTemp);
                          abletagflag[slotTemp-1][portTemp-1]=1;
						  //fprintf(stderr,"TAGslotTemp=%d--portTemp=%d\n",slotTemp,portTemp);
        			  }
                      if(strcmp(v_infoByVID.slot_port_untag[i],"")!=0)
        			  {
                      //fprintf(stderr,"untag[%d]=%s",i,v_infoByVID.slot_port_untag[i]);
                          STemp=strtok(v_infoByVID.slot_port_untag[i],"/");
                          slotTemp=atoi(STemp);
                          PTemp=strtok(NULL,"/"); 
                          portTemp=atoi(PTemp);
                          abletagflag[slotTemp-1][portTemp-1]=2;
						  //fprintf(stderr,"UTAGslotTemp=%d--portTemp=%d\n",slotTemp,portTemp);
        			  }
        		  }
        	  }
              /*这里开始填充可选集*/
              k=0;
         if(0==strcmp(TagTempBuf,"Untag"))
    	  {
        	for(i=0;i<Product_select_info.slot_num;i++)
			{
            	for(j=0;j<Product_select_info.port_num;j++)
				{
					if(0==strcmp(product,"Switch7000"))
						{
							if(2==DefaultUntag[i][j] && flagSlot[i]==1)
            				  {
                                    sprintf(flavors[k],"%d/%d",i+1,j+1);
                    				k++;
            				  }
						}
					else
						{
							if(2==DefaultUntag[i][j])
							{
	                                sprintf(flavors[k],"%d/%d",i+1,j+1);
	    							k++;
							}
						}
                    
				}
			}
			
             ableNumLater=DefuntagNum;
    	  }
          else if(0==strcmp(TagTempBuf,"Tag"))
    	  {
          	Tnum=0;
            result1=show_trunk_list(&head,&Tnum);
            if(Tnum>0)
    	  	{
                q=head.next;
                for(i=0;i<Tnum;i++)
        	  	{
                    result2=show_trunk_byid(q->trunkId,&trunk);
                    if(trunk.masterFlag!=0)
                      {
                        pq=trunk.portHead->next;
                        for(j=0;j<64;j++)
                          {
                            tmpVal[i/32] = (1<<(i%32));
                            if((trunk.mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
                            {
                                abletagflag[pq->slot-1][pq->port-1]=3;
                                //fprintf(stderr,"slot=%d-port=%d",pq->slot,pq->port);
                                //fprintf(stderr,"abletagflag[%d][%d]=%d",pq->slot-1,pq->port-1,abletagflag[pq->slot-1][pq->port-1]);
                            	pq=pq->next;
                            }
                          }
                  	}
        	  	}
                q=q->next;
    	  	}	
               
            	for(i=0;i<Product_select_info.slot_num;i++)
				{
                	for(j=0;j<Product_select_info.port_num;j++)
					{
						if(0==strcmp(product,"Switch7000"))
						{
							if(0==abletagflag[i][j] && flagSlot[i]==1)
            				  {
                                    sprintf(flavors[k],"%d/%d",i+1,j+1);
                					k++;
            				  }
						}
						else
						{
							  if(0==abletagflag[i][j])
	    						{
	                                    sprintf(flavors[k],"%d/%d",i+1,j+1);
	        							k++;
	    						}
						}
                      
					}
				}

              ableNumLater=atoi(numTempLater);
        	  }
		  
  
          result = cgiFormSelectMultiple("can_operation", flavors, ableNumLater, 
          flavorChoicesRight, &invalid);
    	  
          if (result == cgiFormNotFound) 
        	  {
                ShowAlert(search(lcontrol,"Not_Select"));
        	  }
  
          for (i=0; (i < ableNumLater); i++) 
        	  {
              if (flavorChoicesRight[i])
                  { 			   
                   if(0==strcmp(TagTempBuf,"Untag"))
                       addordelete_port(ccgi_dbus_connection,"add",flavors[i],"untag",IDSub,lan);
            	   else
                      addordelete_port(ccgi_dbus_connection,"add",flavors[i],"tag",IDSub,lan);
        		   
        		  }
        	  }
      }
    else if(cgiFormSubmitClicked("delete_port") == cgiFormSuccess)     /*左移删除端口操作*/
      {
        //fprintf(stderr,"1111111111");
          strcpy(TagTempBuf,TagTempLater);
          IDSub=atoi(VID);
          untagNum=0;tagNum=0;ableNum=0;k=0;
          show_vlan_ByID(&v_infoByVID,IDSub,&untagNum,&tagNum);
          for(i=0;i<Product_select_info.port_total_num;i++)
        	  {
                  if(strcmp(v_infoByVID.slot_port_tag[i],"")!=0)
        		  {
                      STemp=strtok(v_infoByVID.slot_port_tag[i],"/");
                      slotTemp=atoi(STemp);
                      PTemp=strtok(NULL,"/"); 
                      portTemp=atoi(PTemp);
                      abletagflag[slotTemp-1][portTemp-1]=1;
        		  }
                  if(strcmp(v_infoByVID.slot_port_untag[i],"")!=0)
        		  {
                      STemp=strtok(v_infoByVID.slot_port_untag[i],"/");
                      slotTemp=atoi(STemp);
                      PTemp=strtok(NULL,"/"); 
                      portTemp=atoi(PTemp);
                      abletagflag[slotTemp-1][portTemp-1]=2;
        		  }
        	  }  
              k=0;
             if(0==strcmp(TagTempBuf,"Untag"))
        	  {
        	   #if 0
                if(0==strcmp(product,"Switch5000"))
        					{

                            	for(i=0;i<1;i++)
                				{
                                	for(j=0;j<24;j++)
                					{
                                        if(2==abletagflag[i][j])
                						{
                                                sprintf(flavors[k],"%d/%d",i+1,j+1);
                    							k++;
                						}
                					}
                				}
        					}
            else if(0==strcmp(product,"Switch7000"))
        		{
                  for(i=0;i<4;i++)
        		  {

                      for(j=0;j<6;j++)
        			  {
                          if(2==abletagflag[i][j])
            				  {
                                    sprintf(flavors[k],"%d/%d",i+1,j+1);
                    				k++;
            				  }
        			  }
        		  	
        		  }
        		 }
			  #endif
			  	for(i=0;i<Product_select_info.slot_num;i++)
				{
                	for(j=0;j<Product_select_info.port_num;j++)
					{
	                        if(2==abletagflag[i][j])
							{
	                                sprintf(flavors[k],"%d/%d",i+1,j+1);
	    							k++;
							}
					}
				}
                  ableNumLater=DefuntagNum;
        	  }
              else if(0==strcmp(TagTempBuf,"Tag"))
        	  {
        	  	#if 0
                if(0==strcmp(product,"Switch5000"))
        					{

                            	for(i=0;i<1;i++)
                				{
                                	for(j=0;j<24;j++)
                					{
                                        if(1==abletagflag[i][j])
                						{
                                                sprintf(flavors[k],"%d/%d",i+1,j+1);
                    							k++;
                						}
                					}
                				}
        					}
            else if(0==strcmp(product,"Switch7000"))
        		{
                  for(i=0;i<4;i++)
        		  {	
                      for(j=0;j<6;j++)
        			  {
                          if(1==abletagflag[i][j])
            				  {
                                    sprintf(flavors[k],"%d/%d",i+1,j+1);
                					k++;
            				  }
        			  }
        		  	
        		  }
        		  }
			  #endif
			  for(i=0; i<Product_select_info.slot_num; i++)
				{
                	for(j=0; j<Product_select_info.port_num; j++)
					{
	                        if (1 == abletagflag[i][j])
							{
	                                sprintf(flavors[k],"%d/%d",i+1,j+1);
	    							k++;
							}
					}
				}

        	 }
          if(0==strcmp(TagTempBuf,"Untag"))
              ableNum=untagNum;
          else if(0==strcmp(TagTempBuf,"Tag"))
              ableNum=tagNum;
  

          result = cgiFormSelectMultiple("havebeen_operation", flavors, ableNum, flavorChoicesLeft, &invalid);
    	 	
          if (result == cgiFormNotFound) 
        	  {
              ShowAlert(search(lcontrol,"Not_Select"));
        	  }
          for (i=0; (i < ableNum); i++) 
        	  {
              if (flavorChoicesLeft[i])
        		  {
                  if(0==strcmp(TagTempBuf,"Untag"))
                      addordelete_port(ccgi_dbus_connection,"delete",flavors[i],"untag",IDSub,lan);
            	   else
                      addordelete_port(ccgi_dbus_connection,"delete",flavors[i],"tag",IDSub,lan);
        		  }
        	  }
    	  
      }

  fprintf(cgiOut,"<form method=post>"\
  "<div align=center>"\
  "<table width=976 border=0 cellpadding=0 cellspacing=0>"\
  "<tr>"\
    "<td width=8 align=left valign=top background=/images/di22.jpg><img src=/images/youce4.jpg width=8 height=30/></td>"\
    "<td width=51 align=left valign=bottom background=/images/di22.jpg><img src=/images/youce33.jpg width=37 height=24/></td>"\
    "<td width=153 align=left valign=bottom background=/images/di22.jpg><font id=titleen>VLAN</font><font id=%s> %s</font></td>",search(lpublic,"title_style"),search(lpublic,"management"));
    fprintf(cgiOut,"<td width=690 align=right valign=bottom background=/images/di22.jpg>");

      fprintf(cgiOut,"<table width=130 border=0 cellspacing=0 cellpadding=0>"\
      "<tr>");

      
      //"<td width=62 align=center><input id=but type=submit name=submit_portconfig style=background-image:url(/images/ok-ch.jpg) value=""></td>");       
       if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
       	{
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,search(lpublic,"img_ok"));
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_vlandetail.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",encry,IDForSearch,search(lpublic,"img_cancel"));
      		
        }
      else
      	{
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_configvlan.cgi?UN=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",port_encry,search(lpublic,"img_ok"));
            fprintf(cgiOut,"<td width=62 align=left><a href=wp_vlandetail.cgi?UN=%s&VID=%s target=mainFrame><img src=/images/%s border=0 width=62 height=20/></a></td>",port_encry,VID,search(lpublic,"img_cancel"));

    	}
      fprintf(cgiOut,"</tr>"\
      "</table>");

    fprintf(cgiOut,"</td>"\
    "<td width=74 align=right valign=top background=/images/di22.jpg><img src=/images/youce3.jpg width=31 height=30/></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=5 align=center valign=middle><table width=976 border=0 cellpadding=0 cellspacing=0 bgcolor=#f0eff0>"\
      "<tr>"\
        "<td width=12 align=left valign=top background=/images/di888.jpg>&nbsp;</td>"\
        "<td width=948><table width=947 border=0 cellspacing=0 cellpadding=0>"\
            "<tr height=4 valign=bottom>"\
              "<td width=120>&nbsp;</td>"\
              "<td width=827 valign=bottom><img src=/images/bottom_05.gif width=827 height=4/></td>"\
            "</tr>"\
            "<tr>"\
              "<td><table width=120 border=0 cellspacing=0 cellpadding=0>"\
                   "<tr height=25>"\
                    "<td id=tdleft>&nbsp;</td>"\
                  "</tr>");
                    fprintf(cgiOut,"<tr height=26>"\
                       "<td align=left id=tdleft background=/images/bottom_bg.gif style=\"border-right:0\"><font id=%s>%s<font></td>",search(lpublic,"menu_san"),search(lcontrol,"port_configure"));   /*突出显示*/
                     fprintf(cgiOut,"</tr>");
                      for(i=0;i<8;i++)
        			  {
                        fprintf(cgiOut,"<tr height=25>"\
                          "<td id=tdleft>&nbsp;</td>"\
                		"</tr>");
        			  }
                  fprintf(cgiOut,"</table>"\
              "</td>"\
              "<td align=left valign=top style=\"background-color:#ffffff; border-right:1px solid #707070; padding-left:30px; padding-top:10px\">"\
                        "<table width=350 height=150 border=0 cellpadding=0 cellspacing=0>"\
                		"<tr>"\
                        "<td align=\"center\" width=140>"\
                        "<table height=120 width=140 border=1 cellspacing=0 cellpadding=0>");
                        if((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && 
                            (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && 
                            (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && 
                            (cgiFormSubmitClicked("del_route") != cgiFormSuccess))
                            IDTemp= atoi(IDForSearch);
            			else 
                        	IDTemp=atoi(VID);
                        memset(v_infoByVID.vlanName,0,21);
                        slotTemp=0;portTemp=0;
						//abletagflag[MAX_SOLT_NUM][MAX_PORT_NUM]; 
                        for(i=0;i<MAX_TOTAL_NUM;i++)
        					{
                                    memset(v_infoByVID.slot_port_tag[i],0,6);
                                    memset(v_infoByVID.slot_port_untag[i],0,6);
                                    memset(VinfoTemp.slot_port_untag[i],0,6);
        					}
                        for(i=0; i<MAX_SOLT_NUM; i++)
        					{
                            	for(j=0; j<MAX_PORT_NUM; j++)
            					{
                                	abletagflag[i][j]=0;  
                                	DefaultUntag[i][j]=0;
            					}
        					}
        				
                        if(CMD_SUCCESS==show_vlan_ByID(&v_infoByVID,IDTemp,&untagNum,&tagNum))
            			{
            			//fprintf(stderr,"TagTempBuf=%s\n",TagTempBuf);
                        if(0==strcmp(TagTempBuf,"Untag"))
        					{
        						
                                show_vlan_ByID(&VinfoTemp, 1, &DefuntagNum, &DeftagNum);
                            	for(i=0; i<Product_select_info.port_total_num; i++)
            					{ 
            						//fprintf(stderr,"VinfoTemp.slot_port_untag[%d]=%s\n",i,VinfoTemp.slot_port_untag[i]);
                                    if(strcmp(VinfoTemp.slot_port_untag[i],"")!=0)
            						{
            							 
                                        UNSTemp=strtok(VinfoTemp.slot_port_untag[i],"/");
                                    	UNslotTemp=atoi(UNSTemp);
                                    	UNPTemp=strtok(NULL,"/"); 
                                    	UNportTemp=atoi(UNPTemp);
                                        DefaultUntag[UNslotTemp-1][UNportTemp-1]=2;
                                        //fprintf(stderr,"DefaultUntag[%d][%d]\n",UNslotTemp-1,UNportTemp-1);
            						}
            					}
        					}
        				
                        	for(i=0; i<Product_select_info.port_total_num; i++)
            					{ 
            						
                                if(strcmp(v_infoByVID.slot_port_tag[i],"")!=0)
            						{
                                    STemp=strtok(v_infoByVID.slot_port_tag[i],"/");
                                	slotTemp=atoi(STemp);
                                    PTemp=strtok(NULL,"/"); 					
                                	portTemp=atoi(PTemp);
                                    abletagflag[slotTemp-1][portTemp-1]=1;
									//fprintf(stderr,"TAGslotTemp=%d--portTemp=%d\n",slotTemp,portTemp);
            						}
                                if(strcmp(v_infoByVID.slot_port_untag[i],"")!=0)
            						{
                                    STemp=strtok(v_infoByVID.slot_port_untag[i],"/");
                                	slotTemp=atoi(STemp);
                                    PTemp=strtok(NULL,"/"); 
                                	portTemp=atoi(PTemp);
                                    abletagflag[slotTemp-1][portTemp-1]=2;
									//fprintf(stderr,"UTAGslotTemp=%d--portTemp=%d\n",slotTemp,portTemp);
            						}
            					}

                            memset (&tmpVal, 0, sizeof(tmpVal));
                            result1=show_trunk_list(&head,&Tnum);
                            if(Tnum>0)
                          	{
                                q=head.next;
                                for(i=0;i<Tnum;i++)
                              	{
                                    result2=show_trunk_byid(q->trunkId,&trunk);
                                    if(trunk.masterFlag!=0)
                                        {
                                          pq=trunk.portHead->next;
                                            for(j=0;j<64;j++)
                                              {
                                                 tmpVal[i/32] = (1<<(i%32));
                                                if((trunk.mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32])
                                                {
                                                    abletagflag[pq->slot-1][pq->port-1]=3;
                                            		pq=pq->next;
                                                }
                                              }
                                	  	}
                              	}
                                q=q->next;
                          	}	
                        if(0==strcmp(TagTempBuf,"Untag"))      /*因为可用untag端口直接查找default vlan就行，对应配置default vlan可用和已用都有,所以应排除*/
        				{
        					
                            fprintf(cgiOut,"<tr height=30>"\
                            "<td style=\"font-size:12px\" bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"can_allocat_untag"));
                            fprintf(cgiOut,"</tr>"\
        				
                			"<tr>"\
                            "<td align=\"center\">");
        			
                            fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\"  name=\"can_operation\" multiple>\n");
                            if(0==strcmp(product,"Switch7000"))
        					{
	                        	for(i=0;i<Product_select_info.slot_num;i++)
	        					{
	                        		for(j=0;j<Product_select_info.port_num;j++)
	            					{
	                                    if(((2==DefaultUntag[i][j]) && (abletagflag[i][j]!=1) && (flagSlot[i]==1)))
	            						{
	                                            fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
	            						}
	            					}
	        					}
	        				}
							else
							{
                            	for(i=0;i<Product_select_info.slot_num;i++)
                				{
                                	for(j=0;j<Product_select_info.port_num;j++)
                					{
                						//fprintf(stderr,"DefaultUntag[%d][%d]=%d--abletagflag[%d][%d]=%d\n",i,j,DefaultUntag[i][j],i,j,abletagflag[i][j]);
                                        if(((2==DefaultUntag[i][j]) && (abletagflag[i][j]!=1)))
                						{
                								
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                                                //fprintf(stderr,"value=\"%d/%d\"",i+1,j+1);
                						}
                					}
                				}
        					}
        				}
                        else if(0==strcmp(TagTempBuf,"Tag"))
        				{
                            fprintf(cgiOut,"<tr height=30>");
    		
                            fprintf(cgiOut,"<td style=\"font-size:12px\" bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"can_allocat_tag"));
                            fprintf(cgiOut,"</tr>"\
        					
                			"<tr>"\
                            "<td align=\"center\">");
        					
                        	fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\"  name=\"can_operation\" multiple>\n");
        					
                            if(0==strcmp(product,"Switch7000"))
        					{
                        	for(i=0;i<Product_select_info.slot_num;i++)
            					{
                            		for(j=0;j<Product_select_info.port_num;j++)
            						{
            							//fprintf(stderr,"abletagflag[%d][%d]=%d\n",i,j,abletagflag[i][j]);
                                        if(0==abletagflag[i][j] && flagSlot[i]==1)
            								{
        			
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
        			 
                        						ableNum++;
            								}
            						}
            					}
        					}
							else
							{

                            	for(i=0;i<Product_select_info.slot_num;i++)
                				{
                                	for(j=0;j<Product_select_info.port_num;j++)
                					{
                						//fprintf(stderr,"abletagflag[%d][%d]=%d\n",i,j,abletagflag[i][j]);
                                        if(0==abletagflag[i][j])
                						{
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                            					ableNum++;
                						}
                					}
                				}
        					}
        				}
                        fprintf(cgiOut, "</select>"\
                		"</td>"\
                		"</tr>"\
        				
                    	"</table>"\
                		"</td>"\
                        "<td align=\"center\">"\
                        "<table width=70 border=0 cellspacing=0 cellpadding=0>"\
                		"<tr>"\
                        "<td align=center>");
                        fprintf(cgiOut,"<input type=submit name=add_port  onclick=\"return confirm('%s')\"  style=\"width:49px;height:49px;background:url(/images/right.gif)  left top no-repeat; border=0 \" value="">",search(lcontrol,"add_port_confirm"));
                        fprintf(cgiOut,"</td>"\
                		"</tr>"\
                		"<tr>"\
                        "<td align=center>");
                        fprintf(cgiOut,"<input type=submit name=delete_port onclick=\"return confirm('%s')\"  style=\"width:49px;height:49px;background:url(/images/left.gif)  left top no-repeat; border=0\"   value="">",search(lcontrol,"delete_port_confirm"));
                        fprintf(cgiOut,"</td>"\
                		"</tr>"\
                    	"</table>"\
                		"</td>"\
        				
                        "<td align=\"center\" width=140 >"\
                        "<table width=140 height=170 border=1 cellspacing=0 cellpadding=0>"\
                        "<tr height=30>");
        				
                        if(0==strcmp(TagTempBuf,"Untag"))
        				{
                        fprintf(cgiOut,"<td style=\"font-size:12px\"  bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"have_allocat_untag"));
                        fprintf(cgiOut,"</tr>"\
                		"<tr>"\
                        "<td align=\"center\" colspan=4>");
                        fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\" name=\"havebeen_operation\" multiple>\n");
        					

                            for(i=0;i<Product_select_info.slot_num;i++)
                			{
                                for(j=0;j<Product_select_info.port_num;j++)
                				{
                                    if(2 == abletagflag[i][j])
                						{
                                            fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						}
                				}
                			}

        				}
            			else
        				{
                        fprintf(cgiOut,"<td style=\"font-size:12px\" bgcolor=#eaeff9 align=\"center\">%s(Slot/Port)</td>",search(lcontrol,"have_allocat_tag"));
                        fprintf(cgiOut,"</tr>"\
                		"<tr>"\
                        "<td align=\"center\" colspan=4>");
                        fprintf(cgiOut, "<select style=\"width:140\" style=\"height:160;background-color:#ebfff9\" name=\"havebeen_operation\" multiple>\n");


                            for(i=0;i<Product_select_info.slot_num;i++)
            				{
                            	for(j=0;j<Product_select_info.port_num;j++)
                				{
                                    if(1==abletagflag[i][j])
                						{
                                            fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						}
                				}
            				}

        			}
                    fprintf(cgiOut, "</select>"\
            		"</td>"\
            		"</tr>");
    				}
                    fprintf(cgiOut,"</table>"\
            		"</td>"\
            	  "</tr>");
        			
                        //Add by qiandawei 2008.07.25
#if 0
                        fprintf(cgiOut,"<tr style=\"padding-left:0px; padding-top:30px\"><td colspan=3 align =left>"\
                        "<table width=350 border=0 cellspacing=0 cellpadding=0>");
                        fprintf(cgiOut,"<tr height=30>");
                          fprintf(cgiOut,"<td width=100>%s:</td>","IGMP SNOOPING");
                          fprintf(cgiOut,"<td width=70><select name=portno style=width:138px>");


                            if(0==strcmp(TagTempBuf,"Untag"))
        					{
                                if(0==strcmp(product,"Switch5000"))
            					{
                            		for(i=0;i<1;i++)
            						{
                            			for(j=0;j<24;j++)
            							{
                                    		if(2==abletagflag[i][j])
                							{
                                                    fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                							}
            							}
            						}
            					}
                                else if(0==strcmp(product,"Switch7000"))
            					{
                                	for(i=0;i<4;i++)
                					{
                                		for(j=0;j<6;j++)
                						{
                                        	if(2==abletagflag[i][j])
                    						{
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                    						}
                						}
                					}
            					}
        					}
            				else
        					{
                                if(0==strcmp(product,"Switch5000"))
            					{
                            		for(i=0;i<1;i++)
            						{
                            			for(j=0;j<24;j++)
            							{
                                    		if(1==abletagflag[i][j])
                							{
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                							}
            							}
            						}
            					}
                                else if(0==strcmp(product,"Switch7000"))
            					{
                                	for(i=0;i<4;i++)
                					{
                                		for(j=0;j<6;j++)
                						{
                                        	if(1==abletagflag[i][j])
                							{
                                                fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                							}
                						}
                					}
            					}
            				}	

                            fprintf(cgiOut,"</select></td>"\
                            "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=submit_igmp style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
                            "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=submit_igmp_disable style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"start"),search(lcontrol,"stop"));
                        fprintf(cgiOut,"</tr></table></td></tr>");


        				
                        char portno[N];
                        memset(portno,0,N);
                        cgiFormStringNoNewlines("portno",portno,N);
                    	int vlanid;
                    	int ret = -1;
                        vlanid = strtoul(VID,0,10);
                        if(cgiFormSubmitClicked("submit_igmp") == cgiFormSuccess)
        				{
                            strcpy(TagTempBuf,TagTempLater);//add 14:10
                            ret = config_igmp_snp_npd_port(vlanid,"enable",portno);//开启vlan端口的igmp功能

                    		switch(ret)
        					{
                				case 0:
                                    ShowAlert(search(lpublic,"oper_succ"));//开启成功
                					break;
                				case 4:
                                    ShowAlert(search(lcontrol,"igmp_no_sta_g"));//全局未开启igmp
                					break;
                				case 8:
                                    ShowAlert(search(lcontrol,"igmp_port_sta"));//此端口已开启igmp
                					break;
                				case 70:
                                    ShowAlert(search(lcontrol,"igmp_no_sta_vlan"));//此VLAN不支持IGMP功能
                        			break;								
                				default:
                                    ShowAlert(search(lpublic,"oper_fail"));//开启失败
        					}
        				}
                        if(cgiFormSubmitClicked("submit_igmp_disable") == cgiFormSuccess)
        				{
                            strcpy(TagTempBuf,TagTempLater);//add 14:10
                            ret = config_igmp_snp_npd_port(vlanid,"disable",portno);//关闭vlan端口的igmp功能
                    		switch(ret)
        					{
                				case 0:
                                    ShowAlert(search(lpublic,"oper_succ"));//关闭成功
                					break;	
                				case 4:
                                    ShowAlert(search(lcontrol,"igmp_no_sta_g"));//全局未开启igmp
                					break;
                				case 7:
                                    ShowAlert(search(lcontrol,"igmp_port_no_sta"));//此端口未开启igmp
                					break;
                				case 70:
                                    ShowAlert(search(lcontrol,"igmp_no_sta_vlan"));//此VLAN不支持IGMP功能
                        			break;								
                				default:
                                    ShowAlert(search(lpublic,"oper_fail"));//开启失败
        					}
        				}


                        fprintf(cgiOut,"<tr style=\"padding-left:0px; padding-top:30px\"><td colspan=3 align =left>"\
                        "<table width=350 border=0 cellspacing=0 cellpadding=0>");
                        fprintf(cgiOut,"<tr height=30>");
                          fprintf(cgiOut,"<td width=100>%s:</td>",search(lcontrol,"route_port"));
                          fprintf(cgiOut,"<td width=70><select name=routeport style=width:138px>");
                          if(0==strcmp(TagTempBuf,"Untag"))
            			  {
                              if(0==strcmp(product,"Switch5000"))
            				  {
                            	  for(i=0;i<1;i++)
            					  {
                            		  for(j=0;j<24;j++)
            						  {
                                    	  if(2==abletagflag[i][j])
                						  {
                                                  fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						  }
            						  }
            					  }
            				  }
                              else if(0==strcmp(product,"Switch7000"))
            				  {
                            	  for(i=0;i<4;i++)
            					  {
                            		  for(j=0;j<6;j++)
            						  {
                                    	  if(2==abletagflag[i][j])
                						  {
                                              fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						  }
            						  }
            					  }
            				  }
            			  }
            			  else
            			  {
                              if(0==strcmp(product,"Switch5000"))
            				  {
                            	  for(i=0;i<1;i++)
            					  {
                            		  for(j=0;j<24;j++)
            						  {
                                    	  if(1==abletagflag[i][j])
                						  {
                                              fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						  }
            						  }
            					  }
            				  }
                              else if(0==strcmp(product,"Switch7000"))
            				  {
                            	  for(i=0;i<4;i++)
            					  {
                            		  for(j=0;j<6;j++)
            						  {
                                    	  if(1==abletagflag[i][j])
                						  {
                                              fprintf(cgiOut, "<option value=\"%d/%d\">%d/%d\n",i+1,j+1,i+1,j+1);
                						  }
            						  }
            					  }
            				  }
            			  }   

                              fprintf(cgiOut,"</select></td>"\
                              "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=add_route style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>"\
                              "<td align=left style=padding-left:10px><input type=submit style=width:50px; height:36px  border=0 name=del_route style=background-image:url(/images/SubBackGif.gif) value=\"%s\"></td>",search(lcontrol,"add_route_port"),search(lcontrol,"del_route_port"));
                          fprintf(cgiOut,"</tr></table></td></tr>");

                        char routeport[N];
                        memset(routeport,0,N);
                        cgiFormStringNoNewlines("routeport",routeport,N);
                    	//int vlanid;
                		ret = -1;
                        //vlanid = strtoul(VID,0,10);
                        if(cgiFormSubmitClicked("add_route") == cgiFormSuccess)
        				{
                            strcpy(TagTempBuf,TagTempLater);//add 14:10
                            ret = igmp_snp_mcroute_port_add_del(vlanid,1,routeport);//添加vlan的路由端口

                    		switch(ret)
        					{
                				case 0:
                                    ShowAlert(search(lpublic,"oper_succ"));
                					break;
                				case 3:
                                    ShowAlert(search(lcontrol,"igmp_port_no_sta"));
                					break;	
                				case 4:
                                    ShowAlert(search(lcontrol,"route_port_exist"));
                    				break;					
                				default:
                                    ShowAlert(search(lpublic,"oper_fail"));
        					}
        				}
                        if(cgiFormSubmitClicked("del_route") == cgiFormSuccess)
        				{
                            strcpy(TagTempBuf,TagTempLater);//add 14:10
                            ret = igmp_snp_mcroute_port_add_del(vlanid,0,routeport);//删除vlan的路由端口
                    		switch(ret)
        					{
                				case 0:
                                    ShowAlert(search(lpublic,"oper_succ"));
                					break;	
                				case 3:
                                    ShowAlert(search(lcontrol,"igmp_port_no_sta"));
                					break;	
                				case 6:
                                    ShowAlert(search(lcontrol,"route_port_no"));
                					break;	
                				case 5:
                                    ShowAlert(search(lcontrol,"no_route_port"));
                					break;	
                				default:
                                    ShowAlert(search(lpublic,"oper_fail"));
        					}
                		}						
        				
            #endif			//end
                        fprintf(cgiOut,"<tr>");
                                        sprintf(numTemp,"%d",ableNum);
                                        if(((cgiFormSubmitClicked("add_port") != cgiFormSuccess) && 
                                            (cgiFormSubmitClicked("delete_port") != cgiFormSuccess)&& (cgiFormSubmitClicked("submit_igmp") != cgiFormSuccess) && 
                                            (cgiFormSubmitClicked("submit_igmp_disable") != cgiFormSuccess)&& (cgiFormSubmitClicked("add_route") != cgiFormSuccess) && 
                                            (cgiFormSubmitClicked("del_route") != cgiFormSuccess)))
            						  {
            						  
                                        fprintf(cgiOut,"<td ><input type=hidden name=encry_port value=%s></td>",encry);
                                        fprintf(cgiOut,"<td ><input type=hidden name=encry_Num value=%s></td>",numTemp);
                                        fprintf(cgiOut,"<td ><input type=hidden name=Tag_Temp value=%s></td>",TagTemp);
                                        fprintf(cgiOut,"<td ><input type=hidden name=VId value=%s></td>",IDForSearch);
            						  }
                					  else
            						  {
                                        fprintf(cgiOut,"<td ><input type=hidden name=encry_port value=%s></td>",port_encry);
                                        fprintf(cgiOut,"<td ><input type=hidden name=encry_Num value=%s></td>",numTempLater);
                                        fprintf(cgiOut,"<td ><input type=hidden name=Tag_Temp value=%s></td>",TagTempLater);
                                        fprintf(cgiOut,"<td ><input type=hidden name=VId value=%s></td>",VID);
            						  }
                                      fprintf(cgiOut,"</tr>"\
                	"</table>"\
              "</td>"\
            "</tr>"\
            "<tr height=4 valign=top>"\
              "<td width=120 height=4 align=right valign=top><img src=/images/bottom_07.gif width=1 height=10/></td>"\
              "<td width=827 height=4 valign=top bgcolor=#FFFFFF><img src=/images/bottom_06.gif width=827 height=15/></td>"\
            "</tr>"\
          "</table>"\
        "</td>"\
        "<td width=15 background=/images/di999.jpg>&nbsp;</td>"\
      "</tr>"\
    "</table></td>"\
  "</tr>"\
  "<tr>"\
    "<td colspan=3 align=left valign=top background=/images/di777.jpg><img src=/images/di555.jpg width=61 height=62/></td>"\
    "<td align=left valign=top background=/images/di777.jpg>&nbsp;</td>"\
    "<td align=left valign=top background=/images/di777.jpg><img src=/images/di666.jpg width=74 height=62/></td>"\
  "</tr>"\
"</table>"\
"</div>"\
"</form>"\
"</body>"\
"</html>");  
for(i=0;i<MAX_TOTAL_NUM;i++)
    {
        free(v_infoByVID.slot_port_tag[i]);
        free(v_infoByVID.slot_port_untag[i]);
        free(VinfoTemp.slot_port_tag[i]);
        free(VinfoTemp.slot_port_untag[i]);
        free(addlistRev[i]);
        free(deletelistRev[i]);
        free(flavors[i]);
    }
free(v_infoByVID.vlanName);
free(VinfoTemp.vlanName);
free(IDForSearch);
free(encry);
free(STemp);
free(PTemp);
free(UNSTemp);
free(UNPTemp);
free(slotNo);
free(portNo);
free(product);
free(encry);
free(sl.modname);
free(sl.sn);
release(lpublic);  
release(lcontrol);
if((result1==1)&&(Tnum>0))
  Free_trunk_head(&head);
if(result2==1)
  Free_trunk_one(&trunk);
return 0;

}
                											 

															 
