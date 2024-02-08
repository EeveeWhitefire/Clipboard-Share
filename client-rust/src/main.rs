extern crate clipboard;
extern crate tokio;
extern crate futures;

use local_ipaddress;
// use std::io::BufReader;
use std::sync::Arc;
use tokio::net::TcpStream;
// use tokio::time::delay_for;
// use tokio::sync::Mutex;
use futures::lock::Mutex;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
// use tokio::prelude::*;

// use std::error::Error;
use clipboard::ClipboardProvider;
use clipboard::ClipboardContext;

// use std::io;
use std::time::Duration;
// use std::io::prelude::*;
// use std::net::TcpStream;
// use std::str;
//

struct ClipboardValueManager
{
    curr_value: String
}

impl ClipboardValueManager
{
    fn read(&self) -> String
    {
       self.curr_value.to_string()
    }

    fn write(&mut self, new_value: &String)
    {
        self.curr_value = new_value.to_string();
    }
}

#[tokio::main(flavor = "multi_thread", worker_threads = 3)]
async fn main() {
    const BUFFER_SIZE:usize = 2048;
    const PORT:i32 = 42069;

    let _ip = local_ipaddress::get().unwrap();

    if let Ok(mut _stream) = TcpStream::connect(format!("{}:{}", _ip, PORT)).await
    {
        println!("Connected to the server!");

        let mut ctx: ClipboardContext = ClipboardProvider::new().unwrap();
        let (mut read, mut write) = tokio::io::split(_stream);
        // let curr_value = ;

        let curr_value_a = Arc::new(Mutex::new(ClipboardValueManager {curr_value: ctx.get_contents().unwrap_or(String::new())}));
        let curr_value_c = Arc::clone(&curr_value_a);

        tokio::spawn(async move {
            let mut t_ctx: ClipboardContext = ClipboardProvider::new().unwrap();
            println!("Starting to listen for changes\n");
            loop 
            {
                let mut buffer = [0u8; BUFFER_SIZE];
                read.read(&mut buffer).await.unwrap();

                let recv_val = std::str::from_utf8(&buffer).unwrap().trim_matches(char::from(0u8)); 

                if recv_val.chars().count() > 0
                {
                    println!("Received value from server: {}", recv_val);
                    
                    let mut value_lock = curr_value_a.lock().await;
                    (*value_lock).write(&recv_val.to_string());
                    std::mem::drop(value_lock);
                    t_ctx.set_contents(recv_val.to_owned()).unwrap();
                }
                // thread::sleep(Duration::from_millis(50));
            }
        });

        println!("Starting event loop");
        loop
        {
            let n_val = ctx.get_contents().unwrap();
            let mut value_lock = curr_value_c.lock().await;
            if (*value_lock).read() != n_val
            {
                (*value_lock).write(&n_val);

                println!("New clipboard value: {}", n_val);
                write.write_all(n_val.as_bytes()).await.unwrap();
                write.flush().await.unwrap();
            }

            std::mem::drop(value_lock);
            tokio::time::sleep(Duration::from_millis(300)).await;
        }
    }
    else 
    {
        println!("Couldn't connect to server");
    }
    
    // Ok(())
}
