
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
		check_flag = 0
		ARGV.each do |i|
			if _line[i] != nil  # not find
				check_flag = 1
			end
		end
		if check_flag == 0
			return
		end
		if _line =~ /test_account:(.*)/
			p $1
			if $1 != nil
				begin
					_s = UDPSocket.new
					_s.bind("10.109.247.207", "5071")
					_s.send($1, 0, "10.109.247.207", "5070")
				ensure
					_s.close
				end
			end
			p 'send deal'
		end
	end

end


test = ExpectTest.new
# '/home/jinguodong/open_project/opensips_1_10/opensips >/dev/tty 2>/dev/null'
# /home/jinguodong/open_project/opensips_1_10/opensips >/tmp/mysips.log 2>/dev/null
# 'tail -f /tmp/mysips.log'
test.test('tail -f /tmp/mysips.log')
#test.test1
