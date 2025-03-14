# ThePiHand
Pi Day 2025 - The Pi Counting Robotic Hand
![Cover-IMG_20250311_121036381](https://github.com/user-attachments/assets/31a66864-1ab4-4c39-a0a9-7a3d109c3e82)

Happy Pi Day 2025 With The Pi Hand!

One of the most celebrated times of the year (I think) is upon us again! It's Pi Day! Each year, the 14th of March (3/14), the world rejoices and celebrates (not really) our beloved natural constant, Pi!

All hail Pi!
![Pi-unrolled-720](https://github.com/user-attachments/assets/f6277844-226d-425b-a49a-47d08fe98664)

https://commons.wikimedia.org/wiki/File:Pi-unrolled-720.gif

More info about Pi here: https://pt.wikipedia.org/wiki/Pi

In previous years I did that:

Github 2022: https://github.com/Montecri/ArduinoPiMachine

LinkedIn 2023: https://www.linkedin.com/pulse/pi-on-pi-happy-pi-day-2023-2nd-generation-machine-cristiano-monteiro/
Github 2023: https://github.com/Montecri/PiOnPi

LinkedIn 2024: https://www.linkedin.com/pulse/pi-py-pi-calculating-pi-python-raspberry-cylon-pepers-monteiro-jtbvc/
Github 2024: https://github.com/Montecri/Pi-Py-Pi

![Pi_pie_(8561590798)](https://github.com/user-attachments/assets/2772964f-4389-4556-b1fd-79eddae2b9b2)
Robert Couse-Baker from Sacramento, California, CC BY 2.0 
 
To celebrate this 2025 Pi Day I decided to create a new "display" to show the digits of Pi one by one. A (sort of) robotic hand, built using SolidWorks, 3D printed, Servo motors powered, which will open and close its "fingers" to display each Pi digit. Digits 0 to 5 range from all fingers closed to all open; the hand will fully close to indicate a new digit from 0 to 5 will be displayed, them display only the fingers associated it; digits from 6 to 9 are displayed as two consecutive digits, for example, 5 + 3 for digit 8, so the hand will show five then quickly show 3, and, you, know, you do the math and reach 8, you get the idea...

![Menor-Cortada-IMG_20250311_112208544_HDR_AE](https://github.com/user-attachments/assets/131326a0-ab72-4c4f-8f03-9fa65f9f4754)


The show is driven by our trusted Arduino Nano, mounted over a special "Servo Shield", which provides convenient connections and power for the SG90/MG90 line of servo motors. Here I'm using MG90 due to the all metal construction. Ruggedized Pi!

That was a good exercise in controlling multiple motors simultaneously. 

![Menor-Cortada-IMG_20250311_133111103_HDR_AE](https://github.com/user-attachments/assets/c8404a05-828d-410f-b4aa-0808bfe1054d)


An external power brick was needed, since the Arduino is unable to provide enough power for a smooth operation for all of them. Also, had to employ some tricks to avoid vibration while the motor is trying to hold a finger up. It's all in the source code, which I repurposed from a previous Pi Day.

I call it: The Pi Hand!

![Menor-Cortada-IMG_20250312_103615718](https://github.com/user-attachments/assets/fd41eea7-1606-4152-9173-3caea0d3f591)

![vlcsnap-2025-03-12-15h47m42s070](https://github.com/user-attachments/assets/f8f6cfc0-a3a7-4e05-8d32-299f59e298bf)


Here is a video of this creation for your enjoyment:

<p align="center"><a href="http://www.youtube.com/watch?feature=player_embedded&v=zKS8LcoMIho" target="_blank">
 <img src="http://img.youtube.com/vi/zKS8LcoMIho/mqdefault.jpg" alt="Watch the video" width="320" border="10" />

All source material to create your own Pi Hand can be found here:

https://github.com/Montecri/ThePiHand

It's crude, but, hey, it works!

- Cristiano


