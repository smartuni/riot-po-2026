JAVA_HOME := /Library/Java/JavaVirtualMachines/temurin-21.jdk/Contents/Home
MAVEN := JAVA_HOME=$(JAVA_HOME) ./mvnw

.PHONY: build compile test run clean

build:
	$(MAVEN) package -DskipTests

compile:
	$(MAVEN) compile

test:
	$(MAVEN) test

run:
	$(MAVEN) spring-boot:run

dev:
	$(MAVEN) spring-boot:run -Dspring-boot.run.profiles=dev

clean:
	$(MAVEN) clean
