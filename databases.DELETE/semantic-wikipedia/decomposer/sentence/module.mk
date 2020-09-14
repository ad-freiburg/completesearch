SRC += sentence/Sentence.cpp sentence/Token.cpp


sentence/SentenceTest: sentence/SentenceTest.cpp sentence/Sentence.o sentence/Token.o util/ContextDecomposerUtil.o base/SemanticWikipediaReader.o
	$(CXX_TEST)  -o $@ $^ $(LINK_OBJS) $(TEST_LIBS) $(TEST_INCLUDE)

