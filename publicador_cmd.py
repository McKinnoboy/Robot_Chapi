#./launch.sh
#ros2 run publicador_python publicador_cmd

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
import serial

class PublicadorCMD(Node):

    def __init__(self):
        super().__init__('publicador_cmd')
        self.publisher_ = self.create_publisher(Twist, '/cmd_vel', 10)
        self.timer_period = 0.01  # 10 ms
        self.timer = self.create_timer(self.timer_period, self.timer_callback)
        self.serial_port = None
        self.linear_speed = 0.0
        self.angular_speed = 0.0

        # Setup serial connection
        try:
            self.serial_port = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.01)  # 10 ms timeout
        except serial.SerialException as e:
            self.get_logger().error('Error opening serial port: %s' % str(e))
            self.serial_port = None

    def timer_callback(self):
        # Leer del puerto serial y ajustar la velocidad lineal o angular según la letra recibida
        if self.serial_port and self.serial_port.in_waiting > 0:
            received_char = self.serial_port.read().decode().strip()
            if received_char == 'a':
                self.get_logger().info('Recibida letra "a". Moviendo rueda derecha.')
                self.linear_speed = 0.2  # No movimiento lineal
                self.angular_speed = 0.3  # Moviendo a la derecha (rueda derecha)
            elif received_char == 'b':
                self.get_logger().info('Recibida letra "b". Moviendo rueda izquierda.')
                self.linear_speed = 0.2  # No movimiento lineal
                self.angular_speed = -0.3  # Moviendo a la izquierda (rueda izquierda)
            elif received_char == 'c':
                self.get_logger().info('Recibida letra "c". Moviendo rueda izquierda.')
                self.linear_speed = 0.0  # No movimiento lineal
                self.angular_speed = 0.9  # Moviendo a la izquierda (rueda izquierda)
            elif received_char == 'd' or received_char == 'e':
                self.get_logger().info('Recibida letra "d" o "e". Parando ambas ruedas.')
                self.linear_speed = 0.0  # Detener el movimiento lineal
                self.angular_speed = 0.0  # Detener el movimiento angular

        # Publicar el mensaje de velocidad
        msg = Twist()
        msg.linear.x = self.linear_speed
        msg.angular.z = self.angular_speed
        self.publisher_.publish(msg)

def main(args=None):
    rclpy.init(args=args)

    publicador_cmd = PublicadorCMD()

    rclpy.spin(publicador_cmd)

    # Destroy the node explicitly
    publicador_cmd.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()