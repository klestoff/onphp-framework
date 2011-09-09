<?php
/***************************************************************************
 *   Copyright (C) 2011 by Evgeny V. Kokovikhin                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

	/**
	 * @ingroup Utils
	**/
	final class MobileGoogleAnalytics
	{
		const VERSION		= '4.4sh';
		
		private $utmGifLocation = "http://www.google-analytics.com/__utm.gif";
		
		private $accountId	= null;
		
		/**
		 * @return MobileGoogleAnalytics
		 */
		public static function create($accountId)
		{
			return new self($accountId);
		}

		public function __construct($accountId)
		{
			$this->accountId = $accountId;
		}
			
		public function getImageUrl(HttpRequest $request)
		{
			$timeStamp = time();
			$domainName =
				$request->hasServerVar('SERVER_NAME')
					? $request->getServerVar('SERVER_NAME')
					: '';
			
			// Get the referrer from the utmr parameter, this is the referrer to the
			// page that contains the tracking pixel, not the referrer for tracking
			// pixel.
			$documentReferer =
				$request->hasServerVar('HTTP_REFERER')
					? urlencode($request->getServerVar('HTTP_REFERER'))
					: '-';
			
			$documentPath =
				$request->hasServerVar('REQUEST_URI')
					? urlencode($request->getServerVar('REQUEST_URI'))
					: '-';
			
			$userAgent =
				$request->hasServerVar("HTTP_USER_AGENT")
					? $request->getServerVar("HTTP_USER_AGENT")
					: '';
			
			// Construct the gif hit url.
			$utmUrl =
				$this->utmGifLocation."?"
				."utmn=".$this->getRandomForGoogle()
				."&utmhn=".urlencode($domainName)
				."&utmr=".urlencode($documentReferer)
				."&utmp=".urlencode($documentPath)
				."&utmac=".$this->accountId;

			return $utmUrl;
		}
		
		private function getRandomForGoogle()
		{
			return rand(0, 0x7fffffff);
		}
	}
?>
