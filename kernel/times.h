struct tms {
	int32_t tms_utime;	/* user time */
	int32_t tms_stime;	/* system time */
	int32_t tms_cutime;	/* user time of children */
	int32_t tms_cstime;	/* system time of children */
};


int32_t times(struct tms *buf);
