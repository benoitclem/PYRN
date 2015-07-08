#!/usr/bin/python

from serial import Serial
from threading import Thread
import Queue
import time
import gtk
from random import random
import gobject

gobject.threads_init()

class EnhancedSerial(Serial):
	def __init__(self, *args, **kwargs):
		#ensure that a reasonable timeout is set
		timeout = kwargs.get('timeout',0.1)
		if timeout < 0.01: timeout = 0.1
		kwargs['timeout'] = timeout
		Serial.__init__(self, *args, **kwargs)
		self.buf = ''

	def readline(self, maxsize=None, timeout=1):
		"""maxsize is ignored, timeout in seconds is the max time that is way for a complete line"""
		tries = 0
		while 1:
			self.buf += self.read(512)
			pos = self.buf.find('\n')
			if pos >= 0:
				line, self.buf = self.buf[:pos+1], self.buf[pos+1:]
				return line
			tries += 1
			if tries * self.timeout > timeout:
				break
		line, self.buf = self.buf, ''
		return line

	def readlines(self, sizehint=None, timeout=1):
		"""read all lines that are available. abort after timout
		when no more data arrives."""
		lines = []
		while 1:
			line = self.readline(timeout=timeout)
			if line:
				lines.append(line)
			if not line or line[-1:] != '\n':
				break
		return lines

class SerialReader(Thread):
	def __init__(self,queue):
		Thread.__init__(self)
		#print("sr: init")
		self.q = queue
		self.ser = EnhancedSerial('/dev/ttyACM0', 115200)
		self.running = True

	def stop(self):
		self.running = False

	def run(self):
		while(self.running):
			#print("sr: loop")
			v = self.ser.readline()
			#print("sr: " +v)
			print(v[:-1])
			self.q.put(v) 
			time.sleep(0.01)
			
class PyApp(gtk.Window): 

	IDLE = 0
	BUS = 1
	ID = 2
	LEN = 3
	DATA = 4

	def __init__(self):
		super(PyApp, self).__init__()

		self.frames = 0;
		self.AScroll = True;

		self.configWindow("sharkan",500,500)

		self.createModel()
		self.createTreeView()
		self.createColumns()

		self.configureSerial()

		self.show_all()

		self.startTime = time.time()
		
		self.memSize = 0x7c00
		self.mainStackSz = 00.0;
		self.threads = []
		for i in range(12):
			self.threads.append([[random(),random(),random(),1.0],[None,None,None]])
		#self.threads = [[[random(),random(),random(),1.0],[None,None,None]]] * 20
		
		self.colors = [[255,0,0],[0,255,0],[0,0,255],[255,255,0],[0,255,255],[255,0,255],[128,128,128],[0,150,161],[125,139,0]]
		self.colorsl = [[255,128,128],[128,255,128],[128,128,255],[255,255,128],[128,255,255],[255,128,255],[255,255,255],[125,139,90]]

	def configWindow(self,title,x,y):
		self.set_size_request(x,y)
		self.set_position(gtk.WIN_POS_CENTER)
		self.connect("destroy",self.quit)
		self.set_title(title)

		self.vbox = gtk.VBox(False, 8)
		
		self.toolbar = gtk.Toolbar()
		self.toolbar.set_style(gtk.TOOLBAR_ICONS)
		self.tbClean = gtk.ToolButton(gtk.STOCK_CLEAR)
		self.tbAutoScroll = gtk.ToggleToolButton(gtk.STOCK_GOTO_BOTTOM)
		self.tbAutoScroll.set_active(self.AScroll)
		self.toolbar.insert(self.tbClean,0)
		self.toolbar.insert(self.tbAutoScroll,1)
		self.vbox.pack_start(self.toolbar, False, False, 0)

		self.tbClean.connect("clicked", self.clean)
		self.tbAutoScroll.connect("clicked", self.autoScroll)
		
		self.area = gtk.DrawingArea()
		self.area.set_size_request(x, 50)
		self.area.connect("expose-event", self.expose)
		self.vbox.pack_start(self.area, False, False, 0)

		self.sw = gtk.ScrolledWindow()
		self.sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		self.sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.vbox.pack_start(self.sw, True, True, 0)

		self.statusbar = gtk.Statusbar()        
		self.vbox.pack_start(self.statusbar, False, False, 0)

		self.add(self.vbox)
		
	def expose(self, widget, event):
		
		cr = widget.window.cairo_create()
		x , y, width, height = event.area
		
		# draw background
		cr.set_source_rgba(0, 0, 0, 1)
		cr.rectangle(0, 0, width, height)
		cr.fill()
		
		
		# draw main stack	
		cr.set_source_rgba(0, 1.0, 0, 1)
		sz = (self.mainStackSz/self.memSize) * width
		cr.rectangle(width - sz, 0, sz, height)
		cr.fill()
		
		# draw other thread stacks
		i = 0
		for t in self.threads:
			if t[1][0] != None:
				di = (float(t[1][0])/self.memSize) * width
				sz = (float(t[1][1])/self.memSize) * width
				st = (float(t[1][2])/self.memSize) * width
				# print di,st,sz
				# Complete Size in random color
				cr.set_source_rgba(t[0][0], t[0][1], t[0][2], 1.0)
				#cr.set_source_rgba(self.colors[i][0], self.colors[i][1], self.colors[i][2], 1)
				cr.rectangle(st, 0, sz, height)
				cr.fill()
				# Occupied size in white
				cr.set_source_rgba(1.0,1.0,1.0,1.0)
				cr.rectangle(st+sz-di, 0, di, height)
				cr.fill()
				i+=1


	def clean(self,v):
		self.store.clear()
		self.frames = 0;
		self.startTime = time.time()
		self.statusbar.push(0, "frames cleared")
		pass

	def	autoScroll(self,v):
		self.AScroll = not self.AScroll
		if self.AScroll:
			self.statusbar.push(0, "autoscroll activated")
		else:
			self.statusbar.push(0, "autoscroll deactivated")
		pass

	def quit(self,v):
		#print("ma: destroy")
		self.sr.stop()
		self.sr.join()
		gtk.main_quit()

	def createModel(self):
		self.store = gtk.ListStore(int, str, str, str, str, str)

	def createTreeView(self):
		self.treeView = gtk.TreeView(self.store)
		#self.treeView.connect("row-activated", self.on_activated)
		self.treeView.connect('size-allocate', self.treeview_changed)
		self.treeView.set_rules_hint(True)
		self.sw.add(self.treeView)

	def createColumns(self):
		rendererText = gtk.CellRendererText()

		column = gtk.TreeViewColumn("i", rendererText, text=0)
		column.set_sort_column_id(0)
		self.treeView.append_column(column)


		column = gtk.TreeViewColumn("time", rendererText, text=1)
		column.set_sort_column_id(1)
		self.treeView.append_column(column)

		column = gtk.TreeViewColumn("bus", rendererText, text=2)
		column.set_sort_column_id(2)
		self.treeView.append_column(column)

		column = gtk.TreeViewColumn("id", rendererText, text=3)
		column.set_sort_column_id(3)
		self.treeView.append_column(column)

		column = gtk.TreeViewColumn("len", rendererText, text=4)
		column.set_sort_column_id(4)
		self.treeView.append_column(column)

		column = gtk.TreeViewColumn("data", rendererText, text=5)
		column.set_sort_column_id(5)
		self.treeView.append_column(column)

	def treeview_changed(self, widget, event, data=None):
		if self.AScroll:
			adj = self.sw.get_vadjustment()
			adj.set_value( adj.upper - adj.page_size )

	def addData(self, d):
		p = ''
		for b in d[3]:
			p += "%02X " %b
		self.store.append([self.frames,str("%.6f"%(time.time()-self.startTime)), str(d[0]), "%02X " %d[1], str(d[2]), p])

		self.statusbar.push(0, "%d frames received" % self.frames)

	"""
	def on_activated(self, widget, row, col):
		model = widget.get_model()
		text = model[row][0] + ", " + model[row][1] + ", " + model[row][2]
		self.statusbar.push(0, text)
	"""

	def configureSerial(self):
		self.q = Queue.Queue()
		self.sr = SerialReader(self.q)
		self.sr.start()

		self.statusbar.push(0, "connected listening ...")
		self.timeoutHandler=gtk.timeout_add(10,self.process)

	def process(self):
		#print("ma: process")
		try:
			d = self.q.get(0)
			if(len(d) != 0):
				try:
					i = d.find('-')
					d0 = d[:i-1].split()[2]
					d1 = d[i+2:]
					if d0 == "CANCommon.cpp":
						p = self.canDbgParse(d1)
						if p != None:
							self.frames+=1
							#print p
							self.addData(p)
					elif (d0 == "MainThread.cpp") or (d0 == "MyOsHelpers.c"):
						p = self.threadParse(d1)
				except:
					pass
					#print(d)
					#print("error parsing")
		except Queue.Empty:
			#print "q is empty"
			pass
		return gtk.TRUE
		
	def threadParse(self,line):
		#print(line)
		sp = line.split()
		if sp[1] == "24:":
			n =  sp[2:]
			if n[0] == "T(001)":
				self.mainStackSz = float(n[11]);
				self.area.queue_draw()
			else:
				#try:
				i = int(n[0][2:-1]) - 2
				#print(i)
				#print(self.threads[i][1])
				self.threads[i][1][0] = float(n[11])
				self.threads[i][1][1] = int(n[7])
				self.threads[i][1][2] = 0x0fffffff & int(n[5],16)
				#self.threads[i][1] = [float(n[11]),int(n[7]),int(n[5],16)]
				#print(self.threads)
				#except:
				#	print("test")
				#	print(n[0][2:-1])

	def canDbgParse(self,line):

		"""
		[DBG] Module CANCommon.cpp - Line 27: SNIFFER RX [2|00000623|4|78 01 5A 94]
		IDLE = 0
		BUS = 1
		ID = 2
		LEN = 3
		DATA = 4
		"""
		rdy = 0
		state = 0
		bus = 0
		canId = 0
		dataLen = 0
		data = []
		val = 0
		curr = 0

		state = self.IDLE

		d = None

		#print("ma: parse " + line)
		for c in line:
			if state == self.IDLE:
				if c == '[':
					rdy += 1
					if rdy == 1:
						state = self.BUS
			elif state == self.BUS:
				if c != '|':
					if (c <= '9') and (c >= '0'):
						bus = bus*10 + int(c)
					else:
						print("ma: bus parseError")
						return None
				else:
					state = self.ID
			elif state == self.ID:
				#print(c)
				if c != '|':
					if ((c <= '9') and (c >= '0')) or ((c <= 'F') and (c >= 'A')) or ((c <= 'f') and (c >= 'd')):
						canId = canId*16 + int(c,16)
					else:
						print("ma: id parseError")
						return None
				else:
					state = self.LEN
			elif state == self.LEN:
				if c != '|':
					if (c <= '9') and (c >= '0'):
						dataLen = dataLen*10 + int(c)
					else:
						print("ma: len parseError")
						return None
				else:
					state = self.DATA
			elif state == self.DATA:
				if (c != ' ') and (c != ']'):
					#print(c)
					if ((c <= '9') and (c >= '0')) or ((c <= 'F') and (c >= 'A')) or  ((c <= 'f') and (c >= 'd')):
						val = val*16 + int(c,16)
					else:
						print("ma: data parseError")
						return None
				elif c != "]":
					data.append(val)
					val = 0
				else:
					data.append(val)
					d = [bus,canId,dataLen,data]
					break
		return d
					
						

PyApp()
gtk.main()
