use std::error::Error;
use std::f32::consts::PI;
use std::fs::File;
use std::cmp::{max, min};
use std::future::Future;
use std::io::{BufWriter, Write};
use std::time::Duration;
use std::thread;

use image::{ImageBuffer, ImageFormat, PixelWithColorType, Rgb, Rgba};
use nokhwa::pixel_format::{RgbAFormat, RgbFormat};
use nokhwa::utils::{CameraFormat, CameraIndex, FrameFormat, RequestedFormat, RequestedFormatType, Resolution};
use nokhwa::Camera;
use reqwest::Response;

const CAMERA_INDEX: u32 = 2;
const CAMERA_FOV: f32 = 43.31;

const BRIGHTNESS_THRESHOLD: u16 = 250;
const N_LEDS: u16 = 450;

struct Matrix3 {
    m_11: f32, m_12: f32, m_13: f32,
    m_21: f32, m_22: f32, m_23: f32,
    m_31: f32, m_32: f32, m_33: f32,
}

impl Matrix3 {
    fn det(&self) -> f32 {
        self.m_11 * (self.m_22 * self.m_33 - self.m_23 * self.m_32) -
        self.m_12 * (self.m_21 * self.m_33 - self.m_23 * self.m_31) +
        self.m_13 * (self.m_21 * self.m_32 - self.m_22 * self.m_31)
    }

    fn inverse(&self) -> Matrix3 {
        let det = self.det();

        Matrix3 {
            m_11: (self.m_22 * self.m_33 - self.m_23 * self.m_32) / det,
            m_12: (self.m_13 * self.m_32 - self.m_12 * self.m_33) / det,
            m_13: (self.m_12 * self.m_23 - self.m_13 * self.m_22) / det,
            m_21: (self.m_23 * self.m_31 - self.m_21 * self.m_33) / det,
            m_22: (self.m_11 * self.m_33 - self.m_13 * self.m_31) / det,
            m_23: (self.m_13 * self.m_21 - self.m_11 * self.m_23) / det,
            m_31: (self.m_21 * self.m_32 - self.m_22 * self.m_31) / det,
            m_32: (self.m_12 * self.m_31 - self.m_11 * self.m_32) / det,
            m_33: (self.m_11 * self.m_22 - self.m_12 * self.m_21) / det,
        }
    }

    fn mult(&self, other: &Vec3) -> Vec3 {
        Vec3 {
            x: self.m_11 * other.x + self.m_12 * other.y + self.m_13 * other.z,
            y: self.m_21 * other.x + self.m_22 * other.y + self.m_23 * other.z,
            z: self.m_31 * other.x + self.m_32 * other.y + self.m_33 * other.z,
        }
    }
}

struct ScreenCoord {
    x: u32,
    y: u32,
}

struct Line3D {
    p: Vec3,
    v: Vec3,
}

#[derive(Debug, Clone)]
struct Vec3 {
    x: f32,
    y: f32,
    z: f32,
}

impl Vec3 {

    fn dot(a: &Vec3, b: &Vec3) -> f32 {
        a.x * b.x + a.y * b.y + a.z * b.z
    }

    fn add(&self, other: &Vec3) -> Vec3 {
        Vec3 {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        }
    }

    fn sub(&self, other: &Vec3) -> Vec3 {
        Vec3 {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        }
    }

    fn sqr_norm(&self) -> f32 {
        self.x * self.x + self.y * self.y + self.z * self.z
    }

    fn norm(&self) -> f32 {
        (self.x * self.x + self.y * self.y + self.z * self.z).sqrt()
    }

    fn normalized(&self) -> Vec3 {
        let norm = self.norm();
        Vec3 {
            x: self.x / norm,
            y: self.y / norm,
            z: self.z / norm,
        }
    }

    fn scale_by(&self, scalar: f32) -> Vec3 {
        Vec3 {
            x: self.x * scalar,
            y: self.y * scalar,
            z: self.z * scalar,
        }
    }

}

struct Vec2 {
    x: f32,
    y: f32,
}

struct LEDCapture {
    theta: f32,
    led_positions: Vec<Option<ScreenCoord>>,
}

struct LocalizerResult {
    led_positions: Vec<Option<Vec3>>,
}


#[tokio::main]
async fn main() {
    //let mut camera = init_camera();

    let mut retake = true;
    //let mut captures: Vec<LEDCapture> = Vec::new();
    let mut dist: f32 = 0.0;
    let mut radius: f32 = 0.0;
    let mut height: f32 = 0.0;
    let mut is_cw = false;

    println!("Enter radius:");
    let mut input = String::new();
    std::io::stdin().read_line(&mut input).unwrap();

    radius = input.trim().parse().unwrap();

    println!("Enter height:");
    input = String::new();
    std::io::stdin().read_line(&mut input).unwrap();

    height = input.trim().parse().unwrap();

    /*
    while retake {
        let capture = capture_leds(&mut camera, dist, is_cw, radius).await;

        let mut n_detected = 0;
        for i in 0..N_LEDS {
            if let Some(_) = &capture.led_positions[i as usize] {
                n_detected += 1;
            }
        }

        captures.push(capture);

        //println!("Detected {} LEDs", n_detected);

        //println!("Would you like to retake the image? (y/n):");
        input = String::new();
        std::io::stdin().read_line(&mut input).unwrap();

        retake = input.trim() == "y";

        if (retake) {
            //println!("Enter new distance:");
            input = String::new();
            std::io::stdin().read_line(&mut input).unwrap();

            dist = input.trim().parse().unwrap();

            //println!("Enter rotation sense (cw/ccw):");
            input = String::new();
            std::io::stdin().read_line(&mut input).unwrap();

            is_cw = input.trim() == "cw";
        }
    }
    */
    let captures = read_captures();

    let mut localizer_result = localize_leds(captures, radius, height, Resolution::new(1080, 1920));

    let mut prev_position = Vec3 { x: 0.0, y: 0.0, z: 0.0 };

    for i in 0..N_LEDS {
        if let Some(postition) = &localizer_result.led_positions[i as usize] {
            prev_position = postition.clone();
        } else {
            let mut j = i;

            while localizer_result.led_positions[j as usize].is_none() {
                j = j + 1;
            }

            let pos_1 = prev_position.clone();
            let pos_2 = localizer_result.led_positions[j as usize].clone().unwrap();

            for k in i..j {
                let t = (k - i) as f32 / (j - i) as f32;
                let x = pos_1.x + (pos_2.x - pos_1.x) * t;
                let y = pos_1.y + (pos_2.y - pos_1.y) * t;
                let z = pos_1.z + (pos_2.z - pos_1.z) * t;

                localizer_result.led_positions[k as usize] = Some(Vec3 { x, y, z });
            }
        }
    }

    for i in 0..N_LEDS {
        if let Some(led) = &localizer_result.led_positions[i as usize] {
            println!("{{{}, {}, {}}},", led.x, led.y, led.z);
        } else {
            println!("{{0, 0, 0}},");
        }
    }
}

fn read_captures() -> Vec<LEDCapture> {
    let contents = std::fs::read_to_string("captures.txt").unwrap();
    let lines: Vec<&str> = contents.lines().collect();

    let mut captures: Vec<LEDCapture> = Vec::new();
    let mut i = 0;

    while i < lines.len() {
        let theta = lines[i].parse::<f32>().unwrap().to_radians();
        i += 1;

        let mut led_positions: Vec<Option<ScreenCoord>> = Vec::new();

        for i in 0..N_LEDS {
            led_positions.push(None);
        }

        while i < lines.len() && lines[i].len() > 1 {
            let tokens = lines[i].split(": ").collect::<Vec<&str>>();

            let led_i = tokens[0].parse::<i32>().unwrap();

            let coords = tokens[1].split(", ").collect::<Vec<&str>>();

            let x = coords[0].parse().unwrap();
            let y = coords[1].parse().unwrap();

            i += 1;

            led_positions[led_i as usize] = Some(ScreenCoord { x, y });
        }

        i += 1;

        captures.push(LEDCapture { theta, led_positions });
    }

    captures
}

fn init_camera() -> Camera {
    let index = CameraIndex::Index(CAMERA_INDEX); 
    let requested: RequestedFormat<'_> = RequestedFormat::new::<RgbFormat>(RequestedFormatType::Closest(CameraFormat::new(Resolution::new(1920, 1080), FrameFormat::MJPEG, 30)));
    let camera = Camera::new(index, requested).unwrap();

    camera
}

fn screen_coord_to_line(coord: &ScreenCoord, radius: f32, height: f32, theta: f32, resolution: Resolution) -> Line3D {
    let half_width = resolution.width() as f32 / 2.0;
    let half_height = resolution.height() as f32 / 2.0;
    let screen_depth = half_width / (CAMERA_FOV / 2.0).to_radians().tan();
    
    let x = coord.x as f32 - half_width;
    let y = half_height - coord.y as f32;

    let alpha = theta - PI / 2.0;

    let p = Vec3 {
        x: alpha.cos() * radius,
        y: alpha.sin() * radius,
        z: height
    };

    let tangent = Vec3 {
        x: theta.cos(),
        y: theta.sin(),
        z: 0.0
    };
    
    let normal = Vec3 {
        x: tangent.y,
        y: -tangent.x,
        z: 0.0
    };

    let tangential_component = tangent.scale_by(x);
    let normal_component = normal.scale_by(screen_depth);
    let vertical_component = Vec3 {
        x: 0.0,
        y: 0.0,
        z: y
    };

    let v = tangential_component.add(&normal_component).add(&vertical_component).normalized();

    Line3D { p, v }
}

//https://stackoverflow.com/questions/48154210/3d-point-closest-to-multiple-lines-in-3d-space
fn least_squares_intersection(lines: Vec<Line3D>) -> Vec3 {
    let mut mat = Matrix3 {
        m_11: 0.0, m_12: 0.0, m_13: 0.0,
        m_21: 0.0, m_22: 0.0, m_23: 0.0,
        m_31: 0.0, m_32: 0.0, m_33: 0.0,
    };

    let mut b = Vec3 { x: 0.0, y: 0.0, z: 0.0 };
    
    for line in lines {
        let p = line.p;
        let v = line.v;

        let row_1 = v.scale_by(v.x).sub(&Vec3 {x: 1.0, y: 0.0, z: 0.0}.scale_by(v.sqr_norm()));
        let row_2 = v.scale_by(v.y).sub(&Vec3 {x: 0.0, y: 1.0, z: 0.0}.scale_by(v.sqr_norm()));
        let row_3 = v.scale_by(v.z).sub(&Vec3 {x: 0.0, y: 0.0, z: 1.0}.scale_by(v.sqr_norm()));

        mat.m_11 += row_1.x;
        mat.m_12 += row_1.y;
        mat.m_13 += row_1.z;

        mat.m_21 += row_2.x;
        mat.m_22 += row_2.y;
        mat.m_23 += row_2.z;

        mat.m_31 += row_3.x;
        mat.m_32 += row_3.y;
        mat.m_33 += row_3.z;

        b = b.add(&v.scale_by(Vec3::dot(&p, &v)));
        b = b.add(&p.scale_by(-v.sqr_norm()));
    }

    let inv = mat.inverse();
    inv.mult(&b)
}

fn localize_leds(captures: Vec<LEDCapture>, radius: f32, height: f32, resolution: Resolution) -> LocalizerResult {
    let mut led_positions = Vec::new();

    for i in 0..(N_LEDS as usize) {
        let mut lines: Vec<Line3D> = Vec::new();

        for capture in &captures {
            if let Some(led) = &capture.led_positions[i] {
                let line = screen_coord_to_line(led, radius, height, capture.theta, resolution);

                lines.push(line);
            }
        }

        if lines.len() < 2 {
            led_positions.push(None);
            continue;
        }

        led_positions.push(Some(least_squares_intersection(lines)));
    }

    LocalizerResult { led_positions }
}

async fn write_led(led_i: u16, state: bool) {
    let client = reqwest::Client::new();

    loop {
        let mut bytes: Vec<u8> = led_i.to_le_bytes().to_vec();
        bytes.push(if state { 255 } else { 0 });
        let n_bytes = bytes.len();

        let res = client.post("http://10.2.6.51/uart")
            .body(bytes)
            .header("Content-Type", "application/octet-stream")
            .header("Content-Length", n_bytes.to_string())
            .send().await;

        match res {
            Ok(_) => break,
            Err(_) => continue,
        }
    }
}

fn set_pixel_grayscale(pixel: &mut Rgb<u8>, a: u8) {
    let Rgb([r, g, b]) = pixel;

    *r = a;
    *g = a;
    *b = a;
}

fn get_brightness(pixel: Rgb<u8>) -> u16 {
    let Rgb([r, g, b]) = pixel;

    let c_max = max(r, max(g, b)) as u16;
    let c_min = min(r, min(g, b)) as u16;
    
    (c_max as u16 + c_min as u16) / 2
}

fn capture_watermark(camera: &mut Camera) {
    let frame = camera.frame().unwrap();
    let mut decoded = frame.decode_image::<RgbFormat>().unwrap();

    decoded.pixels_mut().for_each(|p| {
        if get_brightness(*p) < 200 {
            set_pixel_grayscale(p, 255);
        } else {
            set_pixel_grayscale(p, 0);
        }
    });

    decoded.save("watermark.jpeg").unwrap();
}

async fn capture_leds(camera: &mut Camera, dist: f32, is_cw: bool, radius: f32) -> LEDCapture {
    let mut led_positions = Vec::new();
    
    for i in 0..N_LEDS {
        led_positions.push(capture_led(camera, i).await);
    }

    let mut theta = (dist / (2.0 * radius)).atan() * 2.0;
    
    if is_cw {
        theta *= -1.0;
    }

    LEDCapture { led_positions, theta }
}

fn get_frame(camera: &mut Camera) -> ImageBuffer<Rgb<u8>, Vec<u8>> {
    camera.open_stream().unwrap();

    let frame = camera.frame().unwrap();
    let decoded = frame.decode_image::<RgbFormat>().unwrap();

    camera.stop_stream().unwrap();

    decoded
}

async fn capture_led(camera: &mut Camera, led_i: u16) -> Option<ScreenCoord> {
    write_led(led_i, false).await;
    thread::sleep(Duration::from_millis(100));
    let off_frame = get_frame(camera);

    write_led(led_i, true).await;
    thread::sleep(Duration::from_millis(100));
    let on_frame = get_frame(camera);

    //off_frame.save("off_frame.jpeg").unwrap();
    //on_frame.save("on_frame.jpeg").unwrap();

    let mut avg_x = 0;
    let mut avg_y = 0;
    let mut n_bright = 0;

    (0..off_frame.width()).for_each(|x| {
        (0..off_frame.height()).for_each(|y| {
            let off_pixel = off_frame.get_pixel(x, y);
            let on_pixel = on_frame.get_pixel(x, y);
            
            if get_brightness(*on_pixel) as i32 - get_brightness(*off_pixel) as i32 >= BRIGHTNESS_THRESHOLD as i32 {
                avg_x += x;
                avg_y += y;
                n_bright += 1;
            }
        });
    });

    avg_x /= max(n_bright, 1);
    avg_y /= max(n_bright, 1);

    let tmp_x = avg_x;
    let tmp_y = avg_y;

    //rotate
    avg_x = tmp_y;
    avg_y = off_frame.width() - tmp_x;

    if n_bright <= 10 {
        None
    } else {
        println!("{}: {}, {}", led_i, avg_x, avg_y);
        Some(ScreenCoord { x: avg_x, y: avg_y })
    }
}