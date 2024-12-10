package utils

type Job struct {
	Pid     int
	Status  string
	Cmdargs []string
	Bkgrnd  bool
	JobId   int
}
