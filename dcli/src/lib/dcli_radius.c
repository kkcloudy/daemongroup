#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
//#include "sysdef/returncode.h"
#include "dcli_radius.h"
#include "ws_log_conf.h"
#include "ws_radius_conf.h"
//#include "ws_returncode.h"
#include <sys/wait.h>
#include <ctype.h>


struct cmd_node radius_node = 
{
	RADIUS_NODE,
	"%s(config-radius)# "
};

static struct sql_conf  mysql_conf={
                                {0},
				   {0},
				   {0},
};

int load_radius_sql_conf()
{
	#ifdef __RADIUS
       FILE *fp=NULL;     
       char line[512];
       int iret = MYSQL_ERR;
       int num=0;
	   
	fp=fopen(CONF_RADIUS_MYSQL_D,"r");
       if( NULL != fp )
       {       
	   	  for(num=0;num<3;num++)
	   	  {
	   	         memset( line, 0, sizeof( line) );
                        fgets( line, sizeof(line), fp );
	   	  }
				
	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
//	         printf("test !!!server mysql %s\n",line);                         
		  sscanf( line, "%*s %s", mysql_conf.serverip);
//                printf("test !!!mysql_conf.serverip %s\n",mysql_conf.serverip); 
				
	   	  memset( line, 0, sizeof( line) );
                fgets( line, sizeof(line), fp );
//		  printf("test !!!login %s\n",line);                   
		  sscanf( line, "%*s %s", mysql_conf.login);
//               printf("test !!!mysql_conf.login %s\n",mysql_conf.login); 
				
	   	  memset( line, 0, sizeof( line ) );
                fgets( line, sizeof(line), fp );
//		  printf("test !!!password %s\n",line);                   
		  sscanf( line, "%*s %s", mysql_conf.mysqlpassword);
 //               printf("test !!!mysql_conf.mysqlpassword %s\n",mysql_conf.mysqlpassword); 
		   
		  fclose( fp );
		  fp = NULL;
		  iret = MYSQL_OK;
	}
	else
	{   
            iret = MYSQL_ERR_FILEOPEN_ERR;
	}
	return iret;
#endif
return CMD_SUCCESS;
}

static int wite_radius_sql_conf(  )
{

	#ifdef __RADIUS
     FILE *fp = NULL;
	 int Ret,status;
	 int iret = MYSQL_ERR;
	 char cmd[128];
	 char content[1024];
	 char filecontent[2048];

        if(access(CONF_RADIUS_MYSQL_DIR,0)!=0)
	{
		memset(cmd,0,128);
		sprintf(cmd,"sudo chmod 666 %s",CONF_RADIUS_MYSQL_DIR);
		status=system(cmd);
	}

	 Ret = WEXITSTATUS(status);
	 if(Ret)
	 {
            iret=MYSQL_ERR_FILEOPEN_ERR-1;
	 }

        fp = fopen( CONF_RADIUS_MYSQL_PATH, "w+" );
	if( NULL != fp )
        {       	
	         char sql_begin[128]="sql { \n  database = \"mysql\" \n driver = \"rlm_sql_${database}\" \n";		
	         char db_table[512]="radius_db = \"radius\"\n acct_table1 = \"radacct\"  \n acct_table2 = \"radacct\" \n "
			 	                "postauth_table = \"radpostauth\"\n authcheck_table = \"radcheck\" \n "
			 	                "authreply_table = \"radreply\"\n  groupcheck_table = \"radgroupcheck\" \n groupreply_table = \"radgroupreply\"\n usergroup_table = \"radusergroup\"\n";
		  char sql_end[512]="deletestalesessions = yes \n sqltrace = no \n sqltracefile = ${logdir}/sqltrace.sql \n num_sql_socks = 5 \n"
		  	                        "connect_failure_retry_delay = 60 \n nas_table = \"nas\" \n  $INCLUDE sql/${database}/dialup.conf \n }\n \n";	 
	         strncpy(filecontent,sql_begin,sizeof(sql_begin));
			 
		  memset(content,0,1024);
                sprintf( content,"server= %s \n login= %s \n  password= %s \n",  mysql_conf.serverip, mysql_conf.login, mysql_conf.mysqlpassword);	
                strncat(filecontent,content,sizeof(content));
				
		  memset(content,0,1024);
		  snprintf(content,sizeof(db_table)+sizeof(sql_end),"%s %s ",db_table,sql_end);
		  strncat(filecontent,content,sizeof(content));

		  fwrite(filecontent,strlen(filecontent),1,fp);	
                fclose(fp);
                iret =  MYSQL_OK;
        }
	else
	{
		  iret = MYSQL_ERR_FILEOPEN_ERR;
    }       
    return iret;
	#endif
	return CMD_SUCCESS;
}


DEFUN(conf_radius_func,
	conf_radius_cmd,
	"config radius",
	CONFIG_STR
	"Config radius server!\n"
)
{
	
    char cmd[128];
	if (CONFIG_NODE == vty->node)
	{
		  vty->node = RADIUS_NODE; 
		  #ifdef __RADIUS
       		 if(access(XML_RADIUS_PATH,0)!=0)
            	{
	       		 memset(cmd,0,128);
		 	 sprintf(cmd,"sudo cp %s  %s",XML_RADIUS_D,XML_RADIUS_PATH);
		 	 system(cmd);			 
		        memset(cmd,0,128);
		  	 sprintf(cmd,"sudo chmod 666 %s",XML_RADIUS_PATH);
		  	 system(cmd);
       	         }
        	  if(access(CONF_RADIUS_PATH,0)!=0)
       	        {
		  	memset(cmd,0,128);
		 	sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_D,CONF_RADIUS_PATH);
		 	system(cmd);
		 	memset(cmd,0,128);
		  	sprintf(cmd,"sudo chmod 666 %s",CONF_RADIUS_PATH);
		 	system(cmd);
      		 }
		 if(access(XML_RADIUS_CLIENT_PATH,0)!=0)
            	{
	       		 memset(cmd,0,128);
		 	 sprintf(cmd,"sudo cp %s  %s",XML_RADIUS_CLIENT_D,XML_RADIUS_CLIENT_PATH);
		 	 system(cmd);			 
		        memset(cmd,0,128);
		  	 sprintf(cmd,"sudo chmod 666 %s",XML_RADIUS_CLIENT_PATH);
		  	 system(cmd);
       	         }
		if(access(CONF_RADIUS_CLIENT_PATH,0)!=0)
       	         {
		  	 memset(cmd,0,128);
		 	 sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_CLIENT_D,CONF_RADIUS_CLIENT_PATH);
			 int status=0;                                                 
		 	 status=system(cmd);
//			 vty_out(vty,"the exit status %d\n",status);
			 	
		 	 memset(cmd,0,128);
		  	 sprintf(cmd,"sudo chmod 666 %s",CONF_RADIUS_CLIENT_PATH);
		 	 system(cmd);
      		 }
		#endif
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


/*set the ldap server ip (hostname) in ldap module in radiusd.conf*/
DEFUN(set_radius_ldap_server_func,
	set_radius_ldap_server_cmd,
	"set ldapserver IP IDENTIFY PASSWORD BASEDN",
	"Ldap server (for radius) configuration\n"
	"Set ldap server ip\n"\
	"Ldap server IP address A.B.C.D\n"
	"Ldap identify \n"
	"Password for Identify\n"
	"Suffix DN of backend DataBase for ldap server\n"		
)
{
   #ifdef __RADIUS
//	 vty_out(vty, "%% for test!! input IP parameter : %s  %s %s %s \n", argv[0],argv[1],argv[2],argv[3]);
    if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	//radius enable is : 1 , radius service is enable (default station)
	int radiusflag=-1,ipval=-1,ldap_server_flag=0;	   
 	char *getcontent=(char *)malloc(50);
	memset(getcontent,0,50);
	
        ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                           
//	vty_out(vty,"DEBUG:radiusflag= %d for test the radius service \n",radiusflag);

	if(radiusflag)
	{      
		//vty_out(vty,"radiusflag= %d",radiusflag);
		vty_out(vty,"Radius server is running ,please stop radius service first!\n");
		return CMD_WARNING;
	}

	else if(radiusflag==0)
	{	
//		vty_out(vty," DEBUG:Radius service is disable ,you can config the ldapserver IP now!\n"); 
		char set_ip[50]={0};
		sprintf(set_ip,"server=%s",argv[0]);
		
		char set_identify[128]={0};
		sprintf(set_identify,"identity=%s",argv[1]);

		char set_password[128]={0};
		sprintf(set_password,"password=%s",argv[2]);

		char set_basedn[128]={0};
		sprintf(set_basedn,"basedn=%s",argv[3]);
		
//		vty_out(vty, "IP:=%s  iden:%s  password: %s  basedn:%s\n",set_ip,set_identify,set_password,set_basedn);
		if_radius_exist(); 
		get_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_SERVER, getcontent);
		if(strcmp(getcontent,"")==0)
			{	
				struct ldap_st ldap_head,*dq;
				
       			       int ldapnum=0, freeflag=0;			
				vty_out(vty,"set the ldap server IP \n");
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_SERVER, set_ip);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_IDEN, set_identify);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_PASSWD, set_password);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_BASEDN, set_basedn);
				
       			        read_module_ldap_xml(&ldap_head,& ldapnum);
//				vty_out(vty,"debug!!! the result of read_module_ldap_xml %d\n",ldapnum);
				dq=ldap_head.next;
#if 0
				if(dq!=NULL)
	                      {
			        vty_out(vty,"ldapnum= %d \n",ldapnum);
			        vty_out(vty,"ldap server IP (for radius): ");
			        vty_out(vty , "%s\n",dq->server);
			        vty_out(vty,"identify: ");
			        vty_out(vty , "%s\n",dq->identify);
			   	 vty_out(vty,"ldap password: ");
			   	 vty_out(vty , "%s\n\n",dq->password);	
	       	 	        vty_out(vty,"baseDN: ");
				 vty_out(vty , "%s\n",dq->basedn);
	       		        vty_out(vty,"filter condition: ");
			        vty_out(vty , "%s\n",dq->filter_ldap);			
	        		 vty_out(vty,"access attribute: ");
				 vty_out(vty , "%s\n",dq->access_attr);	
	          		}
#endif
				int write_result=3;
				write_result=write_xml2config_radiusd( CONF_RADIUS_PATH, dq);
			//	vty_out(vty,"write_result=%d\n",write_result);
				
				char *cmd=(char *)malloc(128);
	                      memset(cmd,0,128);
				sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_PATH,CONF_RADIUS_D);
		              system(cmd);
			       free(cmd);
				   
				free_read_ldap_xml(&ldap_head,&freeflag);
				free(getcontent);
				return CMD_SUCCESS;				
			}
		else 
			{
				vty_out(vty,"the ldapserver IP has already been set!please unset the radius ldapservr IP first\n");
				free(getcontent);
				return CMD_WARNING;
			}
	}
	#endif
	return CMD_SUCCESS;
}

/*set the ldap server ip (hostname) in ldap module in radiusd.conf*/
DEFUN(unset_radius_ldap_server_func,
	unset_radius_ldap_server_cmd,
	"unset ldapserver IP ",
	"Ldap server (for radius) configuration\n"
	"Unset ldap server IP\n"\
	"Ldap server IP address A.B.C.D:Get IP define by \"show radius-config-informations\"\n"

)
{
	#ifdef __RADIUS
//	    vty_out(vty, "%% for test input IP parameter : %s !\n", argv[0]);
    if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	// radius service is enable (default state)
	int radiusflag=-1,ipval=-1,ldap_server_flag=0;	   //int syslogflag <==>int radiusflag
 	char *getcontent=(char *)malloc(50);
	memset(getcontent,0,50);
	///////////////////////////////////////////////////////////
      	ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                            
//	vty_out(vty,"radiusflag= %d for test the radius service \n",radiusflag);
//	radiusflag=0;                                                        
	if(radiusflag)
	{
//		vty_out(vty,"radiusflag= %d\n",radiusflag);
		vty_out(vty,"Radius server is running ,please stop radius service first!\n");
		return CMD_WARNING;
	}
	else if(radiusflag==0)
	{	
		char setip[50];
		sprintf(setip,"server=%s",argv[0]);
//		vty_out(vty,"setip=%s DEBUG:Radius service is disable ,you can config the ldapserver IP now!\n",setip);  
		if_radius_exist(); 
		get_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_SERVER, getcontent);
//		vty_out(vty,"the ldap servr IP is %s \n",getcontent);
		if(strcmp(getcontent,setip) == 0 )
			{	
				struct ldap_st ldap_head,*dq;				
       			        int ldapnum=0, freeflag=0;					
//				vty_out(vty,"unset the ldap server IP now !\n");
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_SERVER, EMPTY_CONTENT);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_IDEN, EMPTY_CONTENT);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_PASSWD, EMPTY_CONTENT);
				mod_third_radius_xmlnode(XML_RADIUS_PATH, NODE_LDAP, NODE_BASEDN, EMPTY_CONTENT);
			
				read_module_ldap_xml(&ldap_head,& ldapnum);
//				vty_out(vty,"debug!!! the result of read_module_ldap_xml %d\n",ldapnum);
				dq=ldap_head.next;
#if 0
				if(dq!=NULL)
	                         {
			       vty_out(vty,"ldapnum= %d \n",ldapnum);
			       vty_out(vty,"ldap server IP (for radius): ");
			       vty_out(vty , "%s\n",dq->server);
			       vty_out(vty,"identify: ");
			       vty_out(vty , "%s\n",dq->identify);
			       vty_out(vty,"ldap password: ");
			       vty_out(vty , "%s\n\n",dq->password);	
	       	 	       vty_out(vty,"baseDN: ");
			        vty_out(vty , "%s\n",dq->basedn);
	       		       vty_out(vty,"filter condition: ");
			       vty_out(vty , "%s\n",dq->filter_ldap);			
	        		 vty_out(vty,"access attribute: ");
			       vty_out(vty , "%s\n",dq->access_attr);	
	          		}
#endif
				 int write_result=3;
				write_result=write_xml2config_radiusd( CONF_RADIUS_PATH, dq);
//				vty_out(vty,"DEBUG:write_result=%d\n",write_result);

				char *cmd=(char *)malloc(128);
	                      memset(cmd,0,128);
				sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_PATH,CONF_RADIUS_D);
		               system(cmd);
			       free(cmd);
				
				free_read_ldap_xml(&ldap_head,&freeflag);				
				free(getcontent);
				return CMD_SUCCESS;					
			}
		else 
			{
				vty_out(vty,"The IP address you input is not correct.Get ldapserver IP  by \"show radius-config-informations\"\n");
				free(getcontent);
				return CMD_WARNING;
			}
	}
#endif
return CMD_SUCCESS;
}


DEFUN(contrl_service_func,
	contrl_radiusservice_cmd,
	"service radius (enable|disable)",
	"Radius service control\n"
	"Radius service \n" 
	"Start radius service \n"
	"Stop radius  service \n"
)
{	
	
	if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	#ifdef __RADIUS
	int witch=-1;
	if (!strncmp(argv[0], "enable",strlen(argv[0])))
	{
		witch=1;
	}
	else
	{
		witch=0;
	}
	//FILE *fp=NULL;
	char radius_status[10]={0};
	//if(( fp = fopen( STATUS_RADIUS_PATH ,"w+"))==NULL)
	//{
	//	vty_out(vty, "can not  open radius status file!\n");
	//	goto error_end;
	//}
	char cmd[1024]={0};
	if (witch == 1)
	{
		strcpy(cmd, "sudo /opt/services/init/radius_init start");
		strcpy( radius_status, "enable" );
	}
	else
	{
		strcpy(cmd, "sudo /opt/services/init/radius_init stop");
		strcpy( radius_status, "disable" );
	}
	 int status = system(cmd);	 
   	 int ret = WEXITSTATUS(status);
	if (0 != ret)                                                                                   //system() failed 
	{		
		strcpy( radius_status, (witch)?"disable":"enable" );
		
		//fwrite( radius_status, strlen(radius_status), 1, fp );
		mod_first_radius_xmlnode(XML_RADIUS_PATH, "lstatus", radius_status);                //modify the fist level child node:lstatus 
       		vty_out(vty, "operation is failed ,you should contact administrator!\n");               
	}
	else
	{
		int result_mod=0;
		//fwrite( radius_status, strlen(radius_status), 1, fp );	
		result_mod=mod_first_radius_xmlnode(XML_RADIUS_PATH, "lstatus", radius_status);
//		vty_out(vty,"the result for mod_first_xmlnode is %d ",result_mod);                     
	}
	
	//fflush(fp);
//error_end:	
	//fclose(fp);
	//fp = NULL;
	#endif
	return CMD_SUCCESS;
}



DEFUN(show_radius_info_func,
	show_radius_info_cmd,
	"show radius-config-informations",
	SHOW_STR
	"Radius server configure\n"
)
{
	#ifdef __RADIUS
	if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under radius  mode!\n");
		return CMD_WARNING;
	}
	if_radius_exist();
	if_radius_client_exist();
	vty_out(vty,"=============================================================================================================\n");
	struct ldap_st ldap_head,*dq;
       int ldapnum=0;
       read_module_ldap_xml(&ldap_head,& ldapnum);
	dq=ldap_head.next;
	if(dq!=NULL)
	{
		if(strcmp(dq->server,"")!=0)
		{
			vty_out(vty,"ldap server IP (for radius): ");
			vty_out(vty , "%s\t",dq->server);
			vty_out(vty,"identify: ");
			vty_out(vty , "%s\n",dq->identify);
			vty_out(vty,"ldap password: ");
			vty_out(vty , "%s\t",dq->password);	
	         	vty_out(vty,"baseDN: ");
			vty_out(vty , "%s\n",dq->basedn);

		}
		//dq=dq->next;
	}
//		vty_out(vty,"debug message!\n");
    	if(ldapnum>0)
    	{	int freeflag=0;
    		free_read_ldap_xml(&ldap_head,&freeflag);
    //       vty_out(vty,"debug message!free_flag=%d \n",freeflag);
    	}
	 vty_out(vty,"=============================================================================================================\n");
         int load_result=load_radius_sql_conf();
	  if( load_result == MYSQL_ERR_FILEOPEN_ERR)
        {
	         vty_out(vty,"load_result %d\n",load_result);
	         vty_out(vty,"Get configuration information of Mysql failed!\n");
        }
	 else
	 {
               vty_out(vty,"Mysql server IP (for radius): ");
        	 vty_out(vty , "%s\t",mysql_conf.serverip);
        	 vty_out(vty,"login username: ");
        	 vty_out(vty , "%s\t",mysql_conf.login);
        	 vty_out(vty,"Mysql user password: ");
        	 vty_out(vty , "%s\n",mysql_conf.mysqlpassword);
	 }
	 vty_out(vty,"=============================================================================================================\n");
	 struct radius_client_st  client_head,*cq;
	 int client_num=0;
        read_radius_client_xml(&client_head, &client_num);
//	 vty_out(vty,"DEBUG: total %d client nodes in radius_client_option.xml\n",client_num);
			
	cq=client_head.next;
	while(cq!=NULL)
         {
		  vty_out(vty,"client  IP (for radius): ");
	         vty_out(vty , "%s\t",cq->client_ip);
                vty_out(vty,"secret: ");
	         vty_out(vty , "%s\t",cq->secret);
	         vty_out(vty,"nastype: ");
	         vty_out(vty , "%s\t \n",cq->nastype);
		  cq=cq->next;
		  vty_out(vty,"----------------------------------------------------------------------------------------------------------------\n");
	   }
	if(client_num>0)
	{	
//		vty_out(vty,"debug message!free link list\n");
		int freeflag=0;
		free_read_radius_client_xml(&client_head,&freeflag);
//		vty_out(vty,"debug message!free_flag=%d \n",freeflag);
	}
	vty_out(vty,"=============================================================================================================\n");
	int status=0;
	status=if_radius_enable();
//	vty_out(vty,"debug message:the return value of if_radius_enable() :%d",status);
	if(status==0)
	{
		vty_out(vty,"radius service status is:  disable!\n");
	}
	else
	{
		vty_out(vty,"radius service status is:  enable!\n");
	}
	vty_out(vty,"=============================================================================================================\n");	
	#endif
	return CMD_SUCCESS;
}


/*add the client  ip  in clients.conf*/
DEFUN(add_radius_client_func,
	add_radius_client_cmd,
	"add radius_client IP SECRET NASTYPE",
	"Client (for radius) configuration\n"
	"Set radius client IP\n"\
	"Radius client IP address A.B.C.D\n"
	"Radius secret key (shared with NAS) \n"
	"Nas type (default value:other)\n"	
)
{
	#ifdef __RADIUS
//	vty_out(vty, "%% for test!! input  parameter : %s  %s  %s \n", argv[0],argv[1],argv[2]);
    if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	//test whether radius serviece  enable
	int radiusflag=-1,ipval=-1,client_flag=0;	   

      ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                           
//	vty_out(vty,"radiusflag= %d for test the radius service \n",radiusflag);

	if(radiusflag)
	{
		vty_out(vty,"radiusflag= %d Radius server is enable ,please disable radius service first!\n",radiusflag);
		return CMD_WARNING;
	}

	else if(radiusflag==0)
	{	
//		vty_out(vty," DEBUG:Radius service is disable ,you can config the ldapserver IP now!\n");  
		char set_ip[128]={0};
		sprintf(set_ip,"client   %s",argv[0]);
		
		char set_secret[128]={0};
		sprintf(set_secret,"secret=%s",argv[1]);

		char set_nastype[128]={0};
		sprintf(set_nastype,"nastype=%s",argv[2]);
		
//		vty_out(vty, "IP:=%s  secret: %s  nastype:%s\n",set_ip,set_secret,set_nastype);
		if_radius_client_exist(); 
		client_flag=add_second_node_attr(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0]);
		if(!client_flag)
		{
			struct radius_client_st  client_head,*cq;
			int client_num=0;
//			vty_out(vty,"client_flag=%d, DEBUG:you can add the new client node!\n",client_flag);
			add_radius_client_third_node(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0], "ip", set_ip);
			add_radius_client_third_node(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0], "secret", set_secret);
			add_radius_client_third_node(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0], "nastype", set_nastype);

			read_radius_client_xml(&client_head, &client_num);
//			vty_out(vty,"DEBUG: total %d client nodes in radius_client_option.xml\n",client_num);
#if 0
			cq=client_head.next;
			while(cq!=NULL)
	              {
		
			     vty_out(vty,"client  IP (for radius): ");
			     vty_out(vty , "%s\n",cq->client_ip);
			     vty_out(vty,"secret: ");
			     vty_out(vty , "%s\n",cq->secret);
			     vty_out(vty,"nastype: ");
			     vty_out(vty , "%s\n\n",cq->nastype);
			     cq=cq->next;
	          	   }
#endif			
				int write_result=3;
				write_result=write_xml2config_radiusd_client(CONF_RADIUS_CLIENT_PATH,&client_head);
//				vty_out(vty,"debug message! write_result=%d\n",write_result);

				char *cmd=(char *)malloc(128);
	                      memset(cmd,0,128);
				sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_CLIENT_PATH,CONF_RADIUS_CLIENT_D);
		               system(cmd);
			        free(cmd);
				
				if(client_num>0)
				{	
//					vty_out(vty,"debug message!free link list\n");
					int freeflag=0;
					free_read_radius_client_xml(&client_head,&freeflag);
//					vty_out(vty,"debug message!free_flag=%d \n",freeflag);
				}
				//vty_out(vty,"==========================================================\n");

		}

		else 
			{
				vty_out(vty,"the same client has already been added!\n");				
				return CMD_WARNING;
			}
	}
	#endif
	return CMD_SUCCESS;
}

/*delete the client  ip  in clients.conf*/
DEFUN(delete_radius_client_func,
	delete_radius_client_cmd,
	"delete radius_client IP ",
	"Client (for radius) configuration\n"
	"Delete radius client IP \n"\
	"Radius client IP address A.B.C.D \n"
)
{
	#ifdef __RADIUS
//       vty_out(vty, "%% for test!! input  parameter : %s  \n", argv[0]);
       if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	int radiusflag=-1,ipval=-1,client_flag=0;	   
       ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                           
//	vty_out(vty,"DEBUG:radiusflag= %d for test the radius service \n",radiusflag);

	if(radiusflag)
	{
		vty_out(vty,"radiusflag= %d Radius server is enable ,please disable radius service first!\n",radiusflag);
		return CMD_WARNING;
	}

	else if(radiusflag==0)
	{	
//		vty_out(vty," DEBUG:Radius service is disable ,you can config the ldapserver IP now!\n");  
		char set_ip[128]={0};
		sprintf(set_ip,"client   %s",argv[0]);
		
//		vty_out(vty, "IP:=%s  \n",set_ip);
		if_radius_client_exist(); 
              client_flag=get_radius_second_node_attr(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0]);		
		if(client_flag)
		{
			struct radius_client_st  client_head,*cq;
			int client_num=0;
//			vty_out(vty,"DEBUG:client_flag=%d, you can delete the new client node!\n",client_flag);
			delete_radius_client_second_xmlnode(XML_RADIUS_CLIENT_PATH, NODE_CLIENT, NODE_ATTRIBUTE, argv[0]);
			read_radius_client_xml(&client_head, &client_num);
//			vty_out(vty,"DEBUG: total %d client nodes in radius_client_option.xml\n",client_num);
#if 0			
			cq=client_head.next;
			while(cq!=NULL)
	               {
			     vty_out(vty,"client  IP (for radius): ");
			     vty_out(vty , "%s\n",cq->client_ip);
			     vty_out(vty,"secret: ");
			     vty_out(vty , "%s\n",cq->secret);
			     vty_out(vty,"nastype: ");
			     vty_out(vty , "%s\n\n",cq->nastype);
			     cq=cq->next;
	          	   }
#endif 			
			int write_result=3;
			write_result=write_xml2config_radiusd_client(CONF_RADIUS_CLIENT_PATH,&client_head);
//			vty_out(vty,"debug message! write_result=%d\n",write_result);
			
                      char *cmd=(char *)malloc(128);
	               memset(cmd,0,128);
		        sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_CLIENT_PATH,CONF_RADIUS_CLIENT_D);
		        system(cmd);
			 free(cmd);
			if(client_num>0)
			{	
//				vty_out(vty,"debug message!free link list\n");
				int freeflag=0;
				free_read_radius_client_xml(&client_head,&freeflag);
//				vty_out(vty,"debug message!free_flag=%d \n",freeflag);
			}
			vty_out(vty,"============================================================\n");
		}
		else 
		{
			vty_out(vty,"the client IP you input didn't exists at all ! please show radius-config-informations \n");				
			return CMD_WARNING;
		}
	}
	#endif
	return CMD_SUCCESS;
}

DEFUN(set_radius_mysql_server_func,
	set_radius_mysql_server_cmd,
	"set mysqlserver IP LOGIN PASSWORD",
	"Mysql server (for radius) configuration\n"
	"Set Mysql server for Radius\n"\
	"Mysql server IP address A.B.C.D\n"
	"Mysql login username\n"
	"Mysql user password\n"	
)
{        
	#ifdef __RADIUS
	int radiusflag=-1,ipval=-1,status;
// 	 vty_out(vty, "%% for test!! input  3 parameters : %s  %s %s \n", argv[0],argv[1],argv[2]);
        if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
       
	 ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                           
	if(radiusflag)
	{      
//	       vty_out(vty,"test !!!radiusflag= %d",radiusflag);
		vty_out(vty,"Radius server is running ,please stop radius service first!\n");
		return CMD_WARNING;
	}

	else if(radiusflag==0)
	{	
               if(load_radius_sql_conf() == MYSQL_OK && !strcmp(mysql_conf.serverip,""))
              {		
//            		vty_out(vty," test!!!:Radius service is disable ,you can config the ldapserver IP now!\n"); 
            		int write_result=3;
            		strcpy(mysql_conf.serverip,argv[0]);
            		strcpy(mysql_conf.login,argv[1]);
            		strcpy(mysql_conf.mysqlpassword,argv[2]);
            		write_result=wite_radius_sql_conf();
//            	       vty_out(vty,"test!!write_result=%d\n",write_result);
            				
            		char *cmd=(char *)malloc(128);
            	       memset(cmd,0,128);
            		sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_MYSQL_PATH,CONF_RADIUS_MYSQL_D);
            		status=system(cmd);
            	       free(cmd);
            		if( WEXITSTATUS(status) )
            		{
            		    vty_out(vty,"write the config file failed!");
            		    return  CMD_ERR_NOTHING_TODO;
            		}
            		else
                     	return  CMD_SUCCESS;
               }
	        else 
		{
			vty_out(vty,"the Mysqlserver IP has already been set!please unset the Mysqlservr IP first");
			return CMD_WARNING;
		}
	}
	#endif
	return CMD_SUCCESS;
}


DEFUN(unset_radius_mysql_server_func,
	unset_radius_mysql_server_cmd,
	"unset mysqlserver IP LOGIN PASSWORD",
	"Mysql server (for radius) configuration\n"
	"Unset Mysql server for Radius\n"\
	"Mysql server IP address A.B.C.D\n"
	"Mysql login username\n"
	"Mysql user password\n"	
)
{        
	#ifdef __RADIUS
	int radiusflag=-1,ipval=-1,status;
// 	 vty_out(vty, "%% for test!! input  3 parameters : %s  %s %s \n", argv[0],argv[1],argv[2]);
        if (RADIUS_NODE != vty->node) 
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
       
	 ipval=parse_ip_check(argv[0]);
	if(CMD_SUCCESS != ipval)
	{
		vty_out(vty, "%% Bad parameter : %s !", argv[0]);
		return CMD_WARNING;
	}	

	radiusflag=if_radius_enable();                           
	if(radiusflag)
	{      
//	       vty_out(vty,"test !!!radiusflag= %d",radiusflag);
		vty_out(vty,"Radius server is running ,please stop radius service first!\n");
		return CMD_WARNING;
	}

	else if(radiusflag==0)
	{	
               if(load_radius_sql_conf() == MYSQL_OK && !strcmp(mysql_conf.serverip,argv[0]) && !strcmp(mysql_conf.login,argv[1]) && !strcmp(mysql_conf.mysqlpassword,argv[2]))
              {		
//            		vty_out(vty," test!!!:Radius service is disable ,you can config the ldapserver IP now!\n"); 
            		int write_result=3;
            		strcpy(mysql_conf.serverip,"");
            		strcpy(mysql_conf.login,"");
            		strcpy(mysql_conf.mysqlpassword,"");
            		write_result=wite_radius_sql_conf();
//            	       vty_out(vty,"test!!write_result=%d\n",write_result);
            				
            		char *cmd=(char *)malloc(128);
            	       memset(cmd,0,128);
            		sprintf(cmd,"sudo cp %s %s",CONF_RADIUS_MYSQL_PATH,CONF_RADIUS_MYSQL_D);
            		status=system(cmd);
            	       free(cmd);
            		if( WEXITSTATUS(status) )
            		{
            		    vty_out(vty,"write the config file failed!");
            		    return  CMD_ERR_NOTHING_TODO;
            		}
            		else
                     	return  CMD_SUCCESS;
               }
	        else 
		{
			vty_out(vty,"The Mysqlserver config parameter you input is not correct,Get Mysqlserver parameter  by \"show radius-config-informations\" \n");
			return CMD_WARNING;
		}
	}
	#endif
	return CMD_SUCCESS;
}

int dcli_radiusconf_show_running_config(struct vty* vty)
{    
	#ifdef __RADIUS
    if_radius_exist();
	if_radius_client_exist();
	int ret=-1,optflag = 0,sflag = 0,free_flag=0;
	char cmdstr[256] = {0};
	char newc[20]={0};

	struct ldap_st dst,*dq,*dq1;
	memset(&dst,0,sizeof(dst));
//	 vtysh_add_show_string( "!test Radius section\n" );
	int dnum=0;
       read_module_ldap_xml( &dst, &dnum);                                                  /*read the ldap module in radius.xml*/
	dq1=dst.next;
//	while(dq1!=NULL)
        if(dq1 != NULL &&  (strcmp(dq1->server,"") != 0) )
	{          
		    optflag = 1;
	}
       vtysh_add_show_string( "!Radius section\n" );
	   
       memset(newc,0,20);
      	get_first_radius_xmlnode(XML_RADIUS_PATH, NODE_STATUS,&newc);
	if(strcmp(newc,"")!=0)
	{
	        sflag = 1;	
	}
	if((sflag==1)||(optflag == 1))
	{
		vtysh_add_show_string( "config radius\n" );
	}
	   	
	dq=dst.next;         
       if(NULL!=dq && (strcmp(dq1->server,"") != 0))
       {
        	optflag=1;
		memset(cmdstr,0,256);
		char server[50]={0};
		char identify[128]={0};
		char password[128]={0};
		char basedn[128]={0};
		sscanf(dq->server,"server= %s",server);
		sscanf(dq->identify,"identity= %s",identify);
		sscanf(dq->password,"password= %s",password);
		sscanf(dq->basedn,"basedn= %s",basedn);
		sprintf(cmdstr,"set ldapserver  %s  %s  %s  %s\n",server,identify,password,basedn);
		vtysh_add_show_string(cmdstr);
       }

	if(dnum>0)
	free_read_ldap_xml(&dst, &free_flag);                                          //free the linklist

        struct radius_client_st  client_head,*cq;
	 int client_num=0;
        read_radius_client_xml(&client_head, &client_num);
			
	cq=client_head.next;
	while(cq!=NULL)
         {
		  char client_ip[50]={0};
		  char  secret[128]={0};
		  char  nastype[128]={0};
		
		  sscanf(cq->client_ip,"client %s",client_ip);
		  sscanf(cq->secret,"secret= %s",secret);
		  sscanf(cq->nastype,"nastype = %s",nastype);
		
		  memset(cmdstr,0,256);
	         sprintf(cmdstr,"add radius_client  %s %s %s \n",client_ip,secret,nastype);
		  vtysh_add_show_string(cmdstr);
		  cq=cq->next;
	   }
	if(client_num>0)
	{	
		int freeflag=0;
		free_read_radius_client_xml(&client_head,&freeflag);
	}
//add 2010/10/27	
        if( load_radius_sql_conf() == MYSQL_OK&& strcmp(mysql_conf.serverip,"")&&strcmp(mysql_conf.login,"")&&strcmp(mysql_conf.mysqlpassword,""))
        {
	       memset(cmdstr,0,256);
		sprintf(cmdstr,"set mysqlserver  %s %s % s\n",mysql_conf.serverip,mysql_conf.login,mysql_conf.mysqlpassword);
		vtysh_add_show_string(cmdstr);
        }
//add 2010/10/27	
	if(sflag == 1)
	{
	    	memset(cmdstr,0,256);
		sprintf(cmdstr,"service radius %s\n",newc);
		vtysh_add_show_string(cmdstr);
	}
    	if((optflag==1)||(sflag==1))
	{
		vtysh_add_show_string( "exit\n" );
    	}
//       vtysh_add_show_string( "!test config radius end \n" );
	#endif	
	return CMD_SUCCESS;
}


void dcli_radius_init
(
	void
)  
{
	install_node( &radius_node, dcli_radiusconf_show_running_config, "RADIUS_NODE");
	install_default(RADIUS_NODE);

	install_element(CONFIG_NODE, &conf_radius_cmd);
	install_element(RADIUS_NODE, &contrl_radiusservice_cmd);	
	install_element(RADIUS_NODE, &show_radius_info_cmd);
	install_element(RADIUS_NODE, &set_radius_ldap_server_cmd);
	install_element(RADIUS_NODE, &unset_radius_ldap_server_cmd); 
	install_element(RADIUS_NODE, &add_radius_client_cmd); 
	install_element(RADIUS_NODE, &delete_radius_client_cmd); 
	install_element(RADIUS_NODE, &set_radius_mysql_server_cmd); 
	install_element(RADIUS_NODE, &unset_radius_mysql_server_cmd); 
}

#ifdef __cplusplus
}
#endif


