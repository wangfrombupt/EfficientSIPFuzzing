module MyDebug
	# def included(receiver)
	# 	@my_debug = 1
	# 	puts "included"
	# end
	def initialize(*other)
		if (defined?$DBG) && ($DBG=="on"||$DBG=="ON")
			@my_debug = 1
		end
	end
	def debug_puts(str)
		if @my_debug
			puts str
		end
	end
	def debug_p(str)
		if @my_debug
			p str
		end
	end
	

end	
