package demo;

import java.awt.*;
import javax.swing.*;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;
import java.util.ListIterator;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.BufferedReader;
import java.awt.image.BufferedImage;

import java.io.IOException;
import javax.imageio.ImageIO;


public class VoronoiViewer {
	private static final int WIDTH = 1920;
	private static final int HEIGHT = 1080;
	private List<Color> pixelColorMap;
	private BufferedImage img;

	public VoronoiViewer() {
		this.pixelColorMap = new LinkedList<>();
		this.img = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_INT_RGB );
		readColors();
		colorImage();
	}

	private void readColors() {
		String fileName = "./out/pixelInfo.txt";

		File file = new File(fileName);
		if (file.exists()) {
			try {
			try (BufferedReader br = new BufferedReader(new FileReader(file))) {
        		String line;
        		int r, g, b;
        		String[] rgb;
        		while ((line = br.readLine()) != null) {
        			rgb = line.split(";");
        			r = Integer.valueOf(rgb[0]);
        			g = Integer.valueOf(rgb[1]);
        			b = Integer.valueOf(rgb[2]);
        			this.pixelColorMap.add(new Color(r, g, b));
        		}
    		} catch (Exception e) {
    			e.printStackTrace();
    		}
		} catch (Exception e) {
    		e.printStackTrace();
    	}
    } else {
    	System.out.println("Failed to open pixel info file");
    	System.exit(0);
    }
		
    	
	}
	private void colorImage() {
		Iterator<Color> itr = this.pixelColorMap.listIterator();
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				if (itr.hasNext()) {
					this.img.setRGB(x, y, itr.next().getRGB());
				}
			}
		}
	}
	private void saveImage() {
		try {
			File ouput = new File("./out/Voronoi.png");
			ImageIO.write(this.img, "png", ouput);
			System.out.println("Image saved as: out/Voronoi.png");
		} catch (IOException io) {
			io.printStackTrace();
		}
	}

	public static void main(String[] args) {
		VoronoiViewer vv = new VoronoiViewer();
		vv.saveImage();
	}
}