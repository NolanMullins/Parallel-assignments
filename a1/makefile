obj = ray data task

all: $(obj)

ray: ray.c
	gcc ray.c -o ray -lm

data: data.c
	gcc data.c -o data -lm

task: task.c
	gcc task.c -o task -lm

clean:
	rm $(obj) *.ppm