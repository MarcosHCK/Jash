# Máquina virtual

## Objeto

Descripción abstracta del funcionamiento del objeto que encapsula una máquina virtual. Note que cuando se usan los términos lista o cola es con la intención de asignarle el tipo lista o cola a una variable (como tipo abstracto, ojo).

- **Estado**
  - *pipe_r*: extremo de lectura de la tubería.
  - *pipe_w*: extremo de escritura de la tubería.
  - *proc_in*: archivo de lectura del proceso.
  - *proc_out*: archivo de lectura del proceso.
  - *instructions*: cola de instrucciones.
  - *arguments*: pila de argumentos.
  - *flags*: pila de valores de retorno.
  - *children*: lista de procesos hijos ejecutándose actualmente.
- **API**
  - *.push (instruction)*: agrega *instruction* a *instructions*.
  ```
    function push (self, instruction)
      instructions.push (instruction)
    end
  ```
  - *.execute ()*: ejecuta las instrucciones pendientes en *instructions*. Se espera que este método ejecute todas las instrucciones queden en la cola de instrucciones, y retorne inmediatamente (con valor *true* si la lista de procesos hijos no está vacía o existen instrucciones pendientes, y *false* en caso contrario). Cuando encuentre una instrucción **SYNC** debe detener la ejecución de bytecode adicional (sin retirar la instrucción de la cola) hasta que todos los procesos hijos acaben (aún así el método debe retornar como la haría normalmente, en este caso *true* ya que **SYNC** permanece en la cola).

  ```
    function execute (self)
      if (instructions.peek () == SYNC) then
        for child in children do
          if (child.has_finished () == false) then
            return true
          end
        end

        instructions.pop ();
        return self.execute ();
      else
        while (instructions.is_empty () == false) do
          instruction = instructions.peek ();

          if (instruction == PIPE) then
            .
            .
            .
          if (instruction == SYNC) then
            return true
          else
          if (instruction == IF) then
            .
            .
            .
          end

          instructions.pop ();
        end

        for child in children do
          if (child.has_finished ()) then
            children.remove (child)
          end
        end
      return children.is_empty () == false;
      end
    end
  ```

## Bytecode

Descripción abstracta de las instrucciones que ejecuta la máquina virtual. Note que los pedazos de código explicativo son pseudo-código y no están pensados para representar la implementación.

- Tuberías y redirecciones:
  - **PIPE()**: crea un par de descriptores de archivo que representa una tubería y los guarda en el estado.
  ```
    pipe (&pipe_r, &pipe_w);
  ```
  - **PSI()**: asigna el extremo de lectura de la tubería como archivo de entrada.
  ```
    proc_in = pipe_r;
  ```
  - **PSO()**: asigna el extremo de escritura de la tubería como archivo de salida.
  ```
    proc_out = pipe_w;
  ```
  - **FSI(filename : string)**: asigna el archivo *filename* como archivo de entrada.
  ```
    proc_in = open_r (filename);
  ```
  - **FSO(filename : string)**: asigna el archivo *filename* como archivo de salida.
  ```
    proc_out = open_w (filename);
  ```
- Argumentos y variables:
  - **PAS(value : string)**: apila *value* el la pila de argumentos.
  ```
    arguments.push (value);
  ```
  - **PAP()**: apila el contenido del extremo de lectura de la pila en la pila de argumentos.
  ```
    value = dump (pipe_r);
    arguments.push (value);
  ```
  - **GET(name : string)**: apila el valor de la variable *name* el la pila de argumentos.
  ```
    value = getv (name);
    arguments.push (value);
  ```
  - **SET(name : string)**: sobreescribe el valor de la variable *name* con la cadena en la cima de la pila de argumentos.
  ```
    value = arguments.pop ();
    setv (name, value);
  ```
  - **USET(name : string)**: limpia el valor de la variable *name*.
  ```
    setv (name, NULL);
  ```
  - **DUMP()**: escribe el valor en la cima de la pila de argumentos en el archivo de salida.
  ```
    value = arguments.pop ();
    write (proc_out, value);
  ```
- Procesos y sincronización:
  - **EXEC(command : string)**: ejecuta el proceso *command* usando los archivos de entrada y salida como entrada y salida estándar respectivamente.
  ```
    exec (proc_in, proc_out, command);
  ```
  - **EXECF(command : string)**: similar a **EXEC**, pero adicionalmente apila el valor de retorno del proceso en la pila de valores de retorno.
  ```
    value = exec (proc_in, proc_out, command);
    flags.push (value);
  ```
  - **SYNC()**: sincroniza los procesos en espera, es decir, espera a que los procesos que hay en la lista de procesos terminen.
- Condicionales
  - **IF(instructions : int)**: omite *instructions* instrucciones de la cola si la el valor la cima de la pila de valores de retorno es diferente de 0.
  ```
    value = flags.peek ();

    if (value != 0) then
      for i in range (instructions) do
        instructions.pop ()
      end
    end
  ```
  - **IFN(instructions : int)**: omite *instructions* instrucciones de la cola si la el valor la cima de la pila de valores de retorno es igual a 0.
  ```
    value = flags.peek ();

    if (value == 0) then
      for i in range (instructions) do
        instructions.pop ()
      end
    end
  ```
  - **END()**: marca el final de una secuencia **IF**-**IFN**.
  ```
    flags.pop ();
  ```
  - **LT()**: apila el valor 0 en la pila de valores de retorno (*flags.push (0)*).
  ```
    flags.push (0)
  ```
  - **LF()**: apila el valor 1 en la pila de valores de retorno (*flags.push (1)*).
  ```
    flags.push (1)
  ```
