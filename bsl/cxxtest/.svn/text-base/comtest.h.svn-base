typedef struct _comtest_sysinfo {
	char	vmsize[10];
	char	vmlock[10];
	char	vmstat[10];
	char	vmdata[10];
	char	vmstk[10];
	char	vmexe[10];
	char	vmlib[10];
} comtest_sysinfo;

typedef	enum _comtest_systype {
	vmsize = 0,
	vmlock,
	vmstat,
	vmdata,
	vmstk,
	vmexe,
	vmlib,
	vmend
} comtest_systype;


int		getsysinfo(comtest_sysinfo *sysinfo)
{
	int		procid;
	procid = getpid();
	FILE	*fp;
	char	memfile[100];
	char	strinfo[vmend][10] = {"VmSize", "VmLock", "VmStat", "VmData", "VmStk", "VmExe", "VmLib"};
	char	buff[100], *tmp;
	
	sprintf(memfile, "/proc/%d/status", procid);
	fp = fopen(memfile, "r");
	if (fp == NULL)
		return -1;
	while (fgets(buff, 100, fp))
		{
		for (int i=0; i<vmend; i++)
			if (strstr(buff, strinfo[i]))
				{
				tmp = strchr(buff, ':');
				while (!isdigit(*tmp))
					tmp++;
				switch (i) 
					{
					case	0:
						sprintf(sysinfo->vmsize, "%s", tmp);
						break;
					case	1:
						sprintf(sysinfo->vmlock, "%s", tmp);
						break;
					case	2:
						sprintf(sysinfo->vmstat, "%s", tmp);
						break;
					case	3:
						sprintf(sysinfo->vmdata, "%s", tmp);
						break;
					case	4:
						sprintf(sysinfo->vmstk, "%s", tmp);
						break;
					case	5:
						sprintf(sysinfo->vmexe, "%s", tmp);
						break;
					case	6:
						sprintf(sysinfo->vmlib, "%s", tmp);
						break;
					default	:
						break;
					}
				break;
				}
		}

	fclose(fp);
	return 1;
}

