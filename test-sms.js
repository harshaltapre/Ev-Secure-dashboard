// Test script for SMS functionality
// Run with: node test-sms.js

const testSMS = async () => {
  console.log('Testing SMS functionality...\n')
  
  // Test data
  const testPhoneNumber = '+919322184006' // Admin phone number
  const testOTP = '123456'
  
  try {
    // Test the sendOTP function
    const response = await fetch('http://localhost:5000/api/auth', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        action: 'send_otp',
        phoneNumber: testPhoneNumber
      })
    })
    
    const result = await response.json()
    
    if (result.success) {
      console.log('✅ SMS OTP request successful!')
      console.log('Message:', result.message)
      console.log('\n📱 Check your phone for the SMS message')
      console.log('📱 Phone number:', testPhoneNumber)
    } else {
      console.log('❌ SMS OTP request failed!')
      console.log('Error:', result.message)
    }
    
  } catch (error) {
    console.error('❌ Test failed:', error.message)
    console.log('\nMake sure the server is running on port 5000')
    console.log('Run: npm run dev')
  }
}

// Run the test
testSMS()
