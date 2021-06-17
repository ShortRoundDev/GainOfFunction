.PHONY: clean All

All:
	@echo "----------Building project:[ 4MbToMidnight - Debug ]----------"
	@"$(MAKE)" -f  "4MbToMidnight.mk" && "$(MAKE)" -f  "4MbToMidnight.mk" PostBuild
clean:
	@echo "----------Cleaning project:[ 4MbToMidnight - Debug ]----------"
	@"$(MAKE)" -f  "4MbToMidnight.mk" clean
