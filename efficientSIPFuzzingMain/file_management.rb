# file_management.rb
# consist of many method to manage file or diractory.

module FileManagement
	def initialize
	end
	# write_to_file_force('first_crow/to/1.sip', str)
	def write_to_file_force(_file_name='', _str='', _parent_path='.')
		path = File.join(_parent_path, _file_name)
		p path
		# path = File.expand_path
		dir = File.dirname(path)
		mkdirs(dir)
		name = File.basename(path)
		File.open(path, 'w') do |file|
			file.write _str
		end
	end
	#
	def mkdirs(path)
		if !File.exists? path
			if !mkdirs(File.dirname(path))
				return false
			end
			Dir.mkdir(path)
		end
		return true
	end
end