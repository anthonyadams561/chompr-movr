#!/usr/bin/env ruby

##########################################################################
# chompr movr decodr
#
# This script analyzes the raw accelerometer data from 
# chompr movr to determine changes in sleep orientation
#
# The command line arguments are: 
#   The data file to parse
#   The time of day that the data capture started (in HH:MM:SS format)
# 
# http://chompr.blogspot.com
# anthony.adams.561@gmail.com
#
##########################################################################

require 'time'

# Calculates the standard deviation of a numerical array
def calc_stdev(data)
  # Calculate the mean
  sum = 0
  count = data.length
  data.each do |d|
    sum += d  
  end
  mean = sum / count
    
  # Calculate the square of the difference from the mean, for each value in the array
  sqr_diffs = Array.new
  data.each do |d|
    sqr_diffs.push((d - mean) ** 2)
  end
  
  # Calculate the mean of the squared differences
  # This is the variance
  sum = 0
  count = sqr_diffs.length
  sqr_diffs.each do |d|
    sum += d
  end
  variance = sum / count
  
  # Calculate the square root of the variance
  # This is the standard deviation
  stdev = Math.sqrt(variance)
  
  return stdev
end

# This is a good reference for tilt sensing using a three-axis accelerometer
# Look at equations #25 and #26
# http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf

# The data file should be passed as a command line argument
if ARGV.length == 2
  # Set the start time
  start_time = Time.parse(ARGV[1])
  
  # Initialize some variables
  pos = last_pos = msec_last_transition = nil
  pos_times = Hash.new 0
  msec = 0
  pos = ""
  angles = Array.new
    
  # Loop through each line in the data file
  puts "\nTransitions"
  puts "----------------------------"
  File.readlines(ARGV[0]).each do |line|
    # Skip comment lines
    next if line.start_with? '#'
  
    # Split the lines by comma, into fields
    msec,x,y,z = line.split(',').map(&:to_i)
    
    # Calculate pitch (x) and roll (y) in degrees
    # We are only interested in pitch, if the device is worn horizontally
    pitch = Math.atan2(x, z) * (180 / Math::PI);
    pitch = pitch < 0 ? pitch + 360 : pitch
    
    # Save the pitch for later calculations
    angles.push(pitch)
    
    # You may need roll if your accelerometer is mounted in a different orientation
    roll = Math.atan2(y, z) * (180 / Math::PI);
    roll = roll < 0 ? roll + 360 : roll
    
    # Now that we have the angles, let's figure out what position we're in 
    # This assumes that the device is worn in a certain orientation
    # You may need to adjust this if you didn't copy the design exactly
    #
    # 0 degrees = Back
    # 90 degrees = Left side
    # 180 degrees = Stomach
    # 270 degrees = Right side 
    
    # Save last position 
    last_pos = pos
    
    # Determine current position
    if pitch.between?(45, 135)
      pos = "Left Side"
    elsif pitch.between?(135, 225)
      pos = "Stomach"
    elsif pitch.between?(225, 315)
      pos = "Right Side"
    else
      pos = "Back"
    end
    
    # Calculate the time of this event
    event_time = start_time + msec / 1000
    
    # If the position has changed, print it
    if pos != last_pos
      printf("%-12s %12s\n", pos, event_time.strftime("%l:%M:%S %p"));
      # Assign the elapsed time to this position
      if !msec_last_transition.nil? 
        pos_times[last_pos] += msec - msec_last_transition
      end
      
      # Save this as the last position
      msec_last_transition = msec
    end
  end
  
  # Capture time for final position
  pos_times[pos] += msec - msec_last_transition
  
  # Print a summary, showing the time in each position
  puts "\nSummary"
  puts "----------------------------"
  pos_times.each do |pos_name, milliseconds|
    seconds = milliseconds / 1000.0
    minutes = seconds / 60.0
    hours = minutes / 60.0
    # Print the elapsed time in minutes (or hours)
    if minutes > 59
      printf("%-12s %6.1f hours\n", pos_name, hours);
    else
      printf("%-12s %6.1f minutes\n", pos_name, minutes); 
    end  
  end
  
  # Calculate the standard deviation of the pitch
  # This is a general indication of how much we were moving
  # A larger standard deviation means more movement
  stdev = calc_stdev(angles)
  puts "\nStandard Deviation"
  puts "----------------------------"
  printf("%3.1f degrees\n", stdev);
  
else
  puts "Invalid number of arguments"
end
