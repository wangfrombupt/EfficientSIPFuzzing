
# '/home/jinguodong/open_project/opensips_1_10/opensips >/tmp/mysips.log 2>/dev/null'
# monitor of the opensips, some filter can be set at the beginning of the program, with startup param.
# example:
# 	ruby expect_test.rb test
# 		no filter
# 	ruby expect_test.rb
# 		no output, no usage
# 	ruby expect_test.rb to
# 		filter *to*, just deal with the packet whose name contain 'to'.
#
require 'open3'
require 'socket'
class ExpectTest
	def initialize
	end

	def test(_tmp='')
		if _tmp==''
			raise ArgumentError, "jjkjkjkjkj"
		end
		begin
			p _tmp
			thr = nil
			Open3::popen3("#{_tmp}") do | stdin, stdout, stderr |
				begin
					thr = Thread.new {
						loop {
							_tmp_line = stdout.gets
							deal_stats(_tmp_line)
						}
					}

					thr.join
				rescue Exception
					puts "error do"
				ensure
					puts "popen done"
				end
			end

		rescue Exception => err
			puts err.message
		ensure
			puts "ensure"
		end
	end
	
	def deal_stats(_line='')
		puts _line
		File.open('./expect_text_v1.log', 'a') { |f|
			f << _line
		}
	end

end


test = ExpectTest.new
`rm -f expect_text_v1.log`

# run opensips background, store the output to /tmp/mysips.log.
# run once for one test.
# '/home/jinguodong/open_project/opensips_1_10/opensips >/dev/tty 2>/dev/null'
# 
`rm -f /tmp/mysips.log`
`touch /tmp/mysips.log`
`kill -9 $(ps -ef | grep opensips | awk '{print $2}' )`
`/home/jinguodong/open_project/opensips_1_10/opensips >/tmp/mysips.log 2>/dev/null &`



# catch the result of command of 'tail -f /tmp/mysips.log' to expect_text_v1.log
# test.test('tail -f /tmp/mysips.log')
#test.test1
