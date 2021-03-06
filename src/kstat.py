class ThreadsCommand (gdb.Command):

  def __init__ (self):
    super (ThreadsCommand, self).__init__ ("btos-threadinfo", gdb.COMMAND_USER)
    
  def pid_to_name(self, pid):
  	procCount = gdb.parse_and_eval("proc_processes.dataSize")
  	for pix in range(procCount):
  		cpid = gdb.parse_and_eval("proc_processes.data[%s]->pid" % pix)
  		if cpid == pid: return gdb.parse_and_eval("proc_processes.data[%s]->name.p" % pix)
  	return "Unknown PID"
  	
  def addr_to_module(self, addr):
  	modCount = gdb.parse_and_eval("loaded_modules.dataSize")
  	ret = "KERNEL"
  	for mix in range(modCount):
  		base = gdb.parse_and_eval("loaded_modules.data[%s].elf.mem.aligned" % mix)
  		if base < addr: ret = gdb.parse_and_eval("loaded_modules.data[%s].filename.p" % mix)
  		else: return ret
  	return ret

  def invoke (self, arg, from_tty):
    threadCount = gdb.parse_and_eval("threads.dataSize")
    for tix in range(threadCount):
    	tid = gdb.parse_and_eval("threads.data[%s]->ext_id" % tix)
    	pid = gdb.parse_and_eval("threads.data[%s]->pid" % tix)
    	status = gdb.parse_and_eval("threads.data[%s]->status" % tix)
    	bc = gdb.parse_and_eval("threads.data[%s]->blockcheck" % tix)
    	blockinfo = ""
    	if bc != 0:
    		modname = self.addr_to_module(bc)
    		bcp = gdb.parse_and_eval("threads.data[%s]->bc_param" % tix)
    		blockinfo = "(%s :: %s (%s))" % (modname, bc, bcp)
    	name = self.pid_to_name(pid)
    	print("%s: TID: %s PID: %s (%s) Status: %s %s" % (tix, tid, pid, name, status, blockinfo));

ThreadsCommand ()

class ModsCommand (gdb.Command):
	def __init__ (self):
	   super (ModsCommand, self).__init__ ("btos-modinfo", gdb.COMMAND_USER)
    
	def invoke (self, arg, from_tty):
		modCount = gdb.parse_and_eval("loaded_modules.dataSize")
		for mix in range(modCount):
			base = gdb.parse_and_eval("loaded_modules.data[%s].elf.mem.aligned" % mix)
			name = gdb.parse_and_eval("loaded_modules.data[%s].filename.p" % mix)
			print("%s: Module: %s Addr: %s" % (mix, name, base))

ModsCommand ()

class ProcsCommand (gdb.Command):
	def __init__ (self):
	   super (ProcsCommand, self).__init__ ("btos-procinfo", gdb.COMMAND_USER)
    
	def invoke (self, arg, from_tty):
	  	procCount = gdb.parse_and_eval("proc_processes.dataSize")
	  	for pix in range(procCount):
	  		pid = gdb.parse_and_eval("proc_processes.data[%s]->pid" % pix)
	  		name = gdb.parse_and_eval("proc_processes.data[%s]->name.p" % pix)
	  		status = gdb.parse_and_eval("proc_processes.data[%s]->status" % pix)
	  		print("%s: PID: %s Name: %s Status: %s" % (pix, pid, name, status))

ProcsCommand ()

class ProcCommand (gdb.Command):
	def __init__ (self):
		super (ProcCommand, self).__init__ ("btos-proc", gdb.COMMAND_USER)
	
	def invoke (self, arg, from_tty):
		pix = arg
		print(gdb.parse_and_eval("*proc_processes.data[%s]" % pix))
	  
ProcCommand ()

class ThreadCommand (gdb.Command):
	def __init__ (self):
		super (ThreadCommand, self).__init__ ("btos-thread", gdb.COMMAND_USER)
	
	def invoke (self, arg, from_tty):
		tix = arg
		print(gdb.parse_and_eval("*threads.data[%s]" % tix))
	  
ThreadCommand ()