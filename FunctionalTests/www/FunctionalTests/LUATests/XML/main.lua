xml =  [[<?xml version="1.0" ?><document ><article ><p >This is the first paragraph.</p><h2 class="opt" >Title with opt style</h2></article><article ><p >Some<b >important</b>text.</p></article></document>]]

function run()
	test = mona:fromXML(error,xml)
	
	for index,value in ipairs(test) do
		assert(index <= #test) -- test infinite loop bug on ipairs
	end
	
	assert(test.xml.version == "1.0")
	
	assert(test.document == test[1])
	assert(test.document == test.__value)
	
	assert(test.document.article == test[1][1])
	assert(test.document.article == test[1].__value)
	
	assert(test.document.article.p == test[1][1][1])
	assert(test.document.article.p == test[1][1].__value)

	assert(test.document.article.p.__value=="This is the first paragraph.")
	test.document.article.p.__value="This is the changed paragraph."
	assert(test.document.article.p.__value=="This is the changed paragraph.")
	test.document.article.p.__value="This is the first paragraph."

	assert(mona:toXML(error,test)==xml)
	
end
