file_name = '2_test.file'
p File.file? file_name

require 'fileutils'
FileUtils.touch file_name
File.file? file_name

directory_name = '1_dir'
#FileUtils.mkdir(directory_name)
p File.exists? directory_name

require './create_tree'
create_tree 'mydir' => 
	[{'subdirectory' => [['file_in_sub', 'Jest a simple file.']]}, '.hidden_file', 'ruby_script.rb', 'text_file']

p Dir.entries('mydir')

p File.basename('/home/a/')
p File.dirname('/home/a/')
p File.dirname('aa/b')
p File.expand_path("mydir")
