#!/opt/local/bin/python2.7

from serial import Serial
from threading import Thread
import Queue
import time
import gtk
from random import random
import gobject
import pango 
import copy

gobject.threads_init()

class EnhancedSerial(Serial):
	def __init__(self, *args, **kwargs):
		#ensure that a reasonable timeout is set
		timeout = kwargs.get('timeout',0.1)
		if timeout < 0.01: timeout = 0.1
		kwargs['timeout'] = timeout
		Serial.__init__(self, *args, **kwargs)
		self.flushInput()
		self.buf = ''

	def readlines(self, maxsize=None, timeout=1):
		"""maxsize is ignored, timeout in seconds is the max time that is way for a complete line"""
		self.buf += self.read(1024)
		lines = []
		while 1:
			pos = self.buf.find('\n')
			#print(len(self.buf))
			if pos >= 0:
				lines.append(self.buf[:pos+1])
				self.buf = self.buf[pos+1:]
			else:
				break
		return lines

class SimpleGraph:
	def __init__(self, x,y, nMaxPoints, nBits):
		self.x = x
		self.y = y
		self.marge = 20
		self.maxPoints = nMaxPoints
		self.points = []
		self.resolution = float((2**nBits)-1)
		self.change = True

	def setResolution(self, nBits):
		self.resolution = float((2**nBits)-1)

	def setLabel(self, lbl):
		self.label = lbl

	def drawAxis(self,win,gc):
		# This is the canva
		win.draw_rectangle(gc, False, self.marge, self.marge, self.x-self.marge, self.y-self.marge)

	def drawData(self,win,gc):
		pt = []
		i = 0.0
		gSizeX = self.x-2*self.marge
		gSizeY = self.y-3*self.marge
		npts = len(self.points)
		if npts == 0:
			npts = 1
		for p in self.points:
			pt.append((int(self.marge + (gSizeX * (i/npts))),int(self.y - self.marge - (gSizeY*(p/self.resolution)))))
			i += 1.0
		if len(pt):
			win.draw_lines(gc,pt)

	def draw(self,win,gc):
		#self.drawLabel(win,gc)
		self.drawAxis(win,gc)
		self.drawData(win,gc)


	def appendPoint(self,pt):
		if len(self.points) > self.maxPoints:
			self.points.pop(0)
		self.points.append(pt)
		#print(len(self.points))

class SerialReader(Thread):
	def __init__(self,queue):
		Thread.__init__(self)
		#print("sr: init")
		self.q = queue
		self.ser = EnhancedSerial('/dev/tty.usbmodem1412', 230400)
		self.running = True
		self.calcsFilter = []

	def stop(self):
		self.running = False

	def getNCalc(self):
		return len(self.calcsFilter)

	def addCalc(self, calc):
		if calc in self.calcsFilter:
			self.calcsFilter.append(calc)

	def delCalc(self, calc):
		if calc in self.calcsFilter:
			self.calcsFilter.remove(calc)

	def run(self):
		while(self.running):
			#print("sr: loop")
			v = self.ser.readlines()
			#print(v)
			#print("sr: " +v)
			#print(v[:-1])
			print(self.q.qsize())
			for t in range(len(v)):
				calcId = v[t].strip().replace('|',' ').split(" ")[2]
				#print(f)
				if len(self.calcsFilter) != 0:
					for calc in self.calcsFilter:
						if calc == calcId:
							self.q.put(v[t])
				elif(t%4==0):
					self.q.put(v[t])

	def setFilter(self,calc):
			self.calc = calc

			#time.sleep(0.001)
			
class PyApp(gtk.Window): 

	def __init__(self):
		super(PyApp, self).__init__()

		self.sr = None
		self.configWindow("VariationDetector",1000,700)

		
		self.configureSerial()
		
		self.loopTime = 5
		self.timeoutHandler=gtk.timeout_add(self.loopTime,self.process)

		"""
		#f = open("/Users/clemi/Work/canonair/mbed/Captures/Autre.txt",'r')
		f = open("/Users/clemi/Work/canonair/mbed/Captures/Peugeot407-capture8.txt",'r')
		self.data = f.read().split("\n")
		f.close()

		#print(self.data)
		self.q = Queue.Queue()
		for d in self.data:
			p = d.split(":")[1].strip().replace("[","").replace("]","").replace("|"," ").split(" ")
			#print(p)
			t = "caca %s %s %s" %(p[2],p[3],p[4])
			for i in p[5:]:
				t += ' %s' %i
			#print(t)
			self.q.put(t)
		"""

		self.g = SimpleGraph(700,250,500,8)

		#for i in range(50):
		#	self.g.appendPoint(i*3)

		self.indexStart = 0
		self.iterStart = None
		self.rolling = False
		self.size = 0
		self.framesDisp = {}
		self.frames = {}
		self.framesOld = {}

		# Build The HBOX
		self.hbox = gtk.HBox(False, 3)
		self.add(self.hbox)

		# Build The VBox
		self.vbox1 = gtk.VBox(False,3)
		self.vbox1.set_size_request(200, 700)
		self.hbox.pack_start(self.vbox1,False,False,0);

		# Add the scrolled view
		self.sw = gtk.ScrolledWindow()
		self.sw.set_shadow_type(gtk.SHADOW_ETCHED_IN)
		self.sw.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
		self.sw.set_size_request(200, 550)
		#self.hbox.pack_start(self.sw,False,False,0);
		#self.hbox.add(self.sw)
		self.vbox1.pack_start(self.sw,False,False,0)
		#self.vbox.add(self.sw)

		self.rollingButton = gtk.CheckButton("Rolling")
		self.rollingButton.set_size_request(200, 50)
		self.rollingButton.set_active(False)
		self.rollingButton.unset_flags(gtk.CAN_FOCUS)
		self.rollingButton.connect("clicked", self.onRolling)
		self.vbox1.add(self.rollingButton)

		self.graphButton = gtk.ToggleButton("Add Graph")
		self.graphButton.set_size_request(200, 50)
		self.graphButton.connect("clicked", self.onGraph)
		self.vbox1.add(self.graphButton)

		self.stopButton = gtk.ToggleButton("Stop")
		self.stopButton.set_size_request(200, 50)
		self.stopButton.connect("clicked", self.onStop)
		self.vbox1.add(self.stopButton)

		# Create the store
		self.store = gtk.ListStore(gobject.TYPE_STRING, gobject.TYPE_BOOLEAN)
		self.store.append(["all",False])
		#self.store.append(["240",False])

		self.treeView = gtk.TreeView(self.store)
		#self.treeView.connect("row-activated", self.on_activated)
		self.treeView.set_rules_hint(True)
		self.sw.add(self.treeView)

		# Create the column renderers
		rendererText = gtk.CellRendererText()
		column = gtk.TreeViewColumn("ID", rendererText, text=0)
		column.set_sort_column_id(0)    
		self.treeView.append_column(column)

		rendererToggle = gtk.CellRendererToggle()
		rendererToggle.set_property( 'activatable', True )
		rendererToggle.connect( 'toggled', self.col1_toggled_cb, self.store )
		column = gtk.TreeViewColumn("Visible", rendererToggle, text=0)
		column.add_attribute( rendererToggle, "active", 1)
		column.set_sort_column_id(1)
		self.treeView.append_column(column)

		# Add the Second VBox
		self.vbox2 = gtk.VBox(False,3)
		self.hbox.add(self.vbox2);

		# Add the Text View
		self.entry = gtk.TextView()
		self.entry.set_size_request(200, 400)
		self.entry.add_events(gtk.gdk.KEY_RELEASE_MASK)
		self.entry.connect("copy-clipboard",self.cpyClipBoard)
		self.vbox2.pack_start(self.entry,False,False,0)

		# Add the drawing area
		self.sw2 = gtk.ScrolledWindow()
		self.drawingArea = gtk.DrawingArea()
		self.drawingArea.set_size_request(250, 500)
		self.sw2.add_with_viewport(self.drawingArea)
		self.drawingArea.connect("expose-event", self.area_expose_cb)
		self.vbox2.add(self.sw2)

		# Get the buffer
		self.buffer = self.entry.get_buffer()
		self.c_tag = []
		for i in range(255):
			self.c_tag.append(self.buffer.create_tag( "colored%s"%i, foreground="#%02x0000"%i,weight=pango.WEIGHT_BOLD)) 

		fontdesc = pango.FontDescription("monospace")
		self.entry.modify_font(fontdesc)

		self.show_all()

		self.count = 0;

	def cpyClipBoard(self, udata):
		print(udata)

	def area_expose_cb(self,area,event):
		self.areaStyle = self.drawingArea.get_style()
		self.gc = self.areaStyle.fg_gc[gtk.STATE_NORMAL]
		self.g.draw(self.drawingArea.window,self.gc)
		return True

	def onStop(self,widget):
		lbl = widget.get_label()
		if lbl == "Stop":
			gtk.timeout_remove(self.timeoutHandler)
			widget.set_label("Start")
		elif lbl == "Start":
			widget.set_label("Stop")
			self.timeoutHandler = gtk.timeout_add(self.loopTime,self.process)

	def onRolling(self,widget):
		self.rolling = not self.rolling
		print("caca")

	def onGraph(self,widget):
		self.iterStart, self.iterStop = self.buffer.get_selection_bounds()
		t = self.buffer.get_text(self.iterStart,self.iterStop,False)
		t = t.replace(" ","")
		self.indexStart, self.indexStop = self.iterStart.get_line_index(),self.iterStop.get_line_index()

		self.g.setResolution(len(t))
		self.g.appendPoint(int(t,2))
		#self.startMark, self.stopMark = self.iterStart.get_marks(),self.iterStop.get_marks()
		#print(self.buffer.get_text(self.iterStart,self.iterStop,False))
		#print(self.buffer.get_iter_at_mark(self.mark))
		#print(self.mark.get_buffer())
		#print(self.mark,self.iterStart,self.iterStop)

	def configWindow(self,title,x,y):
		self.set_size_request(x,y)
		self.set_position(gtk.WIN_POS_CENTER)
		self.connect("destroy",self.quit)
		self.set_title(title)

	def col1_toggled_cb( self, cell, path, model ):
		if path == "0":
			gtk.timeout_remove(self.timeoutHandler)
			model[path][1] = not model[path][1]
			if model[path][1]:
				self.loopTime = 10
			else:
				self.loopTime = 5
			self.timeoutHandler = gtk.timeout_add(self.loopTime,self.process)

			for i in range(1,self.size+1):
				print(i,model[path][1])
				model[i][1] = model[path][1]
				self.framesDisp[model[i][0]] = model[path][1]
		else:
			if self.sr:
				if self.sr.getNCalc() == 0:
					model[path][1] = not model[path][1]
					self.framesDisp[model[path][0]] = model[path][1]
					if model[path][1]:
						self.sr.addCalc(model[path][0])
					else:
						self.sr.delCalc(model[path][0])
			else:
				model[path][1] = not model[path][1]
				self.framesDisp[model[path][0]] = model[path][1]
		#self.framesDisp[]
		#print "Toggle '%s' to: %s" % (model[path][0], model[path][1])
		return

	def quit(self,v):
		#print("ma: destroy")
		try:
			if self.sr:
				self.sr.stop()
				self.sr.join()
		except AttributeError:
			pass
		gtk.main_quit()

	def configureSerial(self):
		self.q = Queue.Queue()
		self.sr = SerialReader(self.q)
		self.sr.start()

	def process(self):
		#print("ma: process")
		try:
			d = self.q.get(0)
			if(len(d) != 0):
				f = d.strip().replace('|',' ').split(" ")
				#print(f)
				#try:
				# The second value in array is the bus number
				if (f[1] == "1") or (f[1] == "2"):
					ident = f[2] 
					data = f[4:]
					self.frames[ident] = []
					try:
						for i in data:
							c = []
							for j in range(8):
								# 1111 1111
								if int(i,16)&(1<<(7-j)):
									c.append("1")
								else:
									c.append("0")
							self.frames[ident].append(c)
						# add the ident to frameDisp
						if ident not in self.framesDisp:
							self.framesDisp[ident] = False
							self.store.append([ident,False])
							self.size+=1
						#print(self.framesDisp)
						# check if the frame exist in old
						if ident not in self.framesOld:
							self.framesOld[ident] = []
							for c in self.frames[ident]:
								p = []
								for d in c:
									p.append((d,254))
								self.framesOld[ident].append(p)
						#print(self.framesOld)
						ableToDiff = ident in self.framesOld
						t1 = time.time()
						if self.rolling:
							if self.count >= 30:
								self.buffer.delete(self.buffer.get_start_iter(),self.buffer.get_end_iter())
								self.count = 0
							else:
								self.count += 1
						else:
							self.buffer.delete(self.buffer.get_start_iter(),self.buffer.get_end_iter())
						first = True
						color = None
						chunk = ""
						for identity in self.frames.keys():
							if self.framesDisp[identity]:
								position = self.buffer.get_end_iter()
								self.buffer.insert(position, "%s " % identity) 
								for index in range(len(self.frames[identity])):
									octet = self.frames[identity][index]
									ooctet = self.framesOld[identity][index]
									for k in range(len(octet)):
										# Mean change, so we write the buffer
										if octet[k] != ooctet[k][0]:
											if chunk != "":
												# Insert the black stuffs
												color = self.c_tag[0]
												position = self.buffer.get_end_iter()
												self.buffer.insert_with_tags( position, chunk, color) 
												chunk = ""
											# Insert the color Stuffs
											position = self.buffer.get_end_iter()
											self.buffer.insert_with_tags( position, octet[k], self.c_tag[254]) 
											ooctet[k] = (octet[k],254)
										else:
											if ooctet[k][1] != 0:
												if chunk != "":
													# Insert the black stuffs
													color = self.c_tag[0]
													position = self.buffer.get_end_iter()
													self.buffer.insert_with_tags(position, chunk, color) 
												chunk = ""
												color = self.c_tag[ooctet[k][1]]
												self.buffer.insert_with_tags(position, octet[k], color)
											else:
												chunk += octet[k]
											#self.buffer.insert_with_tags( position, octet[k], color) 
											if ooctet[k][1] > 5:
												ooctet[k] = (ooctet[k][0],ooctet[k][1]-1)
											else:
												ooctet[k] = (ooctet[k][0],0)
									position = self.buffer.get_end_iter()
									if chunk != "":
										#print(chunk)
										# Insert the black stuffs
										color = self.c_tag[0]
										position = self.buffer.get_end_iter()
										self.buffer.insert_with_tags( position, chunk, color) 
										chunk = ""
									self.buffer.insert(position," ") 
								position = self.buffer.get_end_iter()
								self.buffer.insert(position,"\n") 
						t2 = time.time()
						t3 = (t2-t1)*1000
						if self.loopTime<t3:
							print((t3*0.8),self.loopTime)
							print("Warning, could not add to much frames")
						# Do the selection 
						if self.indexStart != 0:
							data = self.buffer.get_text(self.buffer.get_start_iter(),self.buffer.get_end_iter(),False)
							if data:
								t = data[self.indexStart:self.indexStop]
								t = t.replace(" ","")
								self.g.appendPoint(int(t,2))
								self.drawingArea.queue_draw()
					except ValueError as e:
						print("We got a handled error type (1)")
						print(e)
					except IndexError as e:
						print("We got a handelr error type (2)")
				#print(self.frames)
				# save old frames
				#self.framesOld = copy.copy(self.frames)
				#except:
				#	pass
				#print(d)
				
		except Queue.Empty:
			#print "q is empty"
			pass
		except ValueError as e:
			print("We got a handled error type (1)")
			print(e)
		except IndexError as e:
			print("We got a handelr error type (2)")
		return gtk.TRUE
		
						

PyApp()
gtk.main()
