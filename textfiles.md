# Syntax analysis

||PLAN|ERG|MRC|
|---|----|---|---|
|**Non-defining Tags**|`# Comment lines`|`VERSION = 2`|
|||`UNITS = (ENGLISH,METRIC)`|
|**Multi-line Tags**|`=HEADER=` / `=STREAM=`|`[COURSE HEADER]`/`[END COURSE HEADER]`|`[COURSE DATA]`/`[END COURSE DATA]`|`[COURSE TEXT]`/`[END COURSE TEXT]`
||`=INTERVAL=`|`DESCRIPTION =`|
||`=SUBINTERVAL=`|
|**Single-line Tags**|`NAME=`|`FILE NAME = `|
||`DURATION=`|
||`PLAN_TYPE= (0,1)`|
||`WORKOUT_TYPE=`|`MINUTES`\t`WATTS`|`MINUTES`\t`PERCENT`|
||`DESCRIPTION=`|
||`INTERVAL_NAME=`|
||`INTERVAL_DESCRIPTION`|
||`REPEAT=`|
||`PWR_LO`|`FTP =`|
||`PWR_HI`|
||`PERCENT_FTP_LO`|
||`PERCENT_FTP_HI`|
||`HR_HI`|
||`HR_LO`|
||`MESG_DURATION_SEC>=<int>`|
||`TSS =`|
||`IF = `|