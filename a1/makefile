obj = ray data task

all: $(obj)

ray: ray.c
	gcc ray.c -o ray -lm -std=c99

data: data.c
	gcc data.c -o data -lm -pthread -std=c99

task: task.c
	gcc task.c -o task -lm -pthread -std=99

clean:
	rm $(obj) *.ppm