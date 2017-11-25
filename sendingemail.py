import yagmail
import smtplib


yag = yagmail.SMTP('Enter Email Here','Enter Your Password Here' )
yag.send(contents = "Enter the message that you want to send to yourself via email here")




