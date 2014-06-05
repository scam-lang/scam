#src = $(wildcard *.scm)
#src = agenda.scm arrays.scm adders.scm a.scm \
      block.scm \
	  call.scm catch.scm chain.scm common.scm conditional.scm \

src=`cat goldlist`

export PATH := .:$(PATH)

tests:
	testgold $(src)
	vi differences

clean:
	@rm -f test/* ./output/* gmon.out differences junk

jeff:
	@touch main.lib
	@scam2 jeff1.scm > output/out1
	@scam2 jeff2.scm > output/out2
	@scam2 jeff3.scm > output/out3
	@scam2 jeff4.scm > output/out4
	@rm main.lib
